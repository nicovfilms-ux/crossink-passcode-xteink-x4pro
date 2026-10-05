#include "PinStorage.h"

#include <HalStorage.h>
#include <mbedtls/sha256.h>

#include <cctype>
#include <cstdio>
#include <cstring>

#ifndef SIMULATOR
#include <esp_random.h>
#include <esp_system.h>
#else
#include <cstdlib>
#endif

#include "CrossPointSettings.h"
#include "Logging.h"

bool PinStorage::hasPin() {
  return SETTINGS.pinHash[0] != '\0' && SETTINGS.pinSalt[0] != '\0';
}

bool PinStorage::isLockEnabled() {
  return (SETTINGS.deviceLockEnabled != 0) && hasPin();
}

bool PinStorage::verifyPin(const char* pin) {
  if (!hasPin() || pin == nullptr) return false;
  const size_t len = strlen(pin);
  if (len < MIN_PIN_LEN || len > MAX_PIN_LEN) return false;

  char computed[HASH_LEN + 1];
  computeHash(SETTINGS.pinSalt, pin, computed, sizeof(computed));

  // Constant-time comparison to prevent timing attacks
  const size_t storedLen = strlen(SETTINGS.pinHash);
  if (storedLen != HASH_LEN) return false;

  int diff = 0;
  for (size_t i = 0; i < HASH_LEN; ++i) {
    diff |= (computed[i] ^ SETTINGS.pinHash[i]);
  }
  return (diff == 0);
}

bool PinStorage::setPin(const char* pin) {
  if (pin == nullptr) return false;
  const size_t len = strlen(pin);
  if (len < MIN_PIN_LEN || len > MAX_PIN_LEN) return false;

  for (size_t i = 0; i < len; ++i) {
    if (!std::isdigit(static_cast<unsigned char>(pin[i]))) return false;
  }

  char salt[SALT_LEN + 1];
  generateSalt(salt, sizeof(salt));

  char hash[HASH_LEN + 1];
  computeHash(salt, pin, hash, sizeof(hash));

  strncpy(SETTINGS.pinSalt, salt, sizeof(SETTINGS.pinSalt) - 1);
  SETTINGS.pinSalt[sizeof(SETTINGS.pinSalt) - 1] = '\0';

  strncpy(SETTINGS.pinHash, hash, sizeof(SETTINGS.pinHash) - 1);
  SETTINGS.pinHash[sizeof(SETTINGS.pinHash) - 1] = '\0';

  SETTINGS.pinLength = (len == 6) ? 6 : 4;
  SETTINGS.deviceLockEnabled = 1;
  const bool saved = SETTINGS.saveToFile();
  LOG_INF("PIN", "New PIN configured and saved (status: %d)", saved);
  return saved;
}

void PinStorage::clearPin() {
  SETTINGS.deviceLockEnabled = 0;
  SETTINGS.pinHash[0] = '\0';
  SETTINGS.pinSalt[0] = '\0';
  SETTINGS.saveToFile();
  LOG_INF("PIN", "PIN cleared and lock disabled");
}

bool PinStorage::checkAndApplySdRecovery() {
  constexpr const char* resetFiles[] = {"/.crosspoint/pin_reset.txt", "/pin_reset.txt"};
  for (const char* path : resetFiles) {
    if (Storage.exists(path)) {
      LOG_INF("PIN", "Recovery marker found at %s! Resetting PIN lock.", path);
      clearPin();
      Storage.remove(path);
      return true;
    }
  }
  return false;
}

bool PinStorage::hasSecurityQuestion() {
  return SETTINGS.securityAnswerHash[0] != '\0' && SETTINGS.securityAnswerSalt[0] != '\0';
}

uint8_t PinStorage::getSecurityQuestion() {
  return SETTINGS.securityQuestion;
}

std::string PinStorage::normalizeAnswer(const char* answer) {
  if (answer == nullptr) return "";

  // Skip leading whitespace
  const char* start = answer;
  while (*start && std::isspace(static_cast<unsigned char>(*start))) {
    ++start;
  }
  if (*start == '\0') return "";

  // Skip trailing whitespace
  const char* end = start + strlen(start) - 1;
  while (end > start && std::isspace(static_cast<unsigned char>(*end))) {
    --end;
  }

  std::string normalized;
  normalized.reserve(end - start + 1);
  for (const char* p = start; p <= end; ++p) {
    const unsigned char c1 = static_cast<unsigned char>(*p);
    if (c1 == 0xC3 && (p + 1) <= end) {
      const unsigned char c2 = static_cast<unsigned char>(*(p + 1));
      if (c2 == 0xA1 || c2 == 0x81) { normalized.push_back('a'); ++p; continue; }  // á, Á
      if (c2 == 0xA9 || c2 == 0x89) { normalized.push_back('e'); ++p; continue; }  // é, É
      if (c2 == 0xAD || c2 == 0x8D) { normalized.push_back('i'); ++p; continue; }  // í, Í
      if (c2 == 0xB3 || c2 == 0x93) { normalized.push_back('o'); ++p; continue; }  // ó, Ó
      if (c2 == 0xBA || c2 == 0x9A || c2 == 0xBC || c2 == 0x9C) { normalized.push_back('u'); ++p; continue; }  // ú, Ú, ü, Ü
    }
    normalized.push_back(static_cast<char>(std::tolower(c1)));
  }
  return normalized;
}

bool PinStorage::setSecurityQuestion(const uint8_t questionIdx, const char* answer) {
  if (answer == nullptr) return false;
  const std::string normalized = normalizeAnswer(answer);
  if (normalized.empty()) return false;

  char salt[SALT_LEN + 1];
  generateSalt(salt, sizeof(salt));

  char hash[HASH_LEN + 1];
  computeHash(salt, normalized.c_str(), hash, sizeof(hash));

  SETTINGS.securityQuestion = questionIdx;
  strncpy(SETTINGS.securityAnswerSalt, salt, sizeof(SETTINGS.securityAnswerSalt) - 1);
  SETTINGS.securityAnswerSalt[sizeof(SETTINGS.securityAnswerSalt) - 1] = '\0';

  strncpy(SETTINGS.securityAnswerHash, hash, sizeof(SETTINGS.securityAnswerHash) - 1);
  SETTINGS.securityAnswerHash[sizeof(SETTINGS.securityAnswerHash) - 1] = '\0';

  const bool saved = SETTINGS.saveToFile();
  LOG_INF("PIN", "Security question %u configured and saved (status: %d)", questionIdx, saved);
  return saved;
}

bool PinStorage::verifySecurityAnswer(const char* answer) {
  if (!hasSecurityQuestion() || answer == nullptr) return false;
  const std::string normalized = normalizeAnswer(answer);
  if (normalized.empty()) return false;

  char computed[HASH_LEN + 1];
  computeHash(SETTINGS.securityAnswerSalt, normalized.c_str(), computed, sizeof(computed));

  const size_t storedLen = strlen(SETTINGS.securityAnswerHash);
  if (storedLen != HASH_LEN) return false;

  int diff = 0;
  for (size_t i = 0; i < HASH_LEN; ++i) {
    diff |= (computed[i] ^ SETTINGS.securityAnswerHash[i]);
  }
  return (diff == 0);
}

void PinStorage::generateSalt(char* outSalt, const size_t maxLen) {
  if (outSalt == nullptr || maxLen <= SALT_LEN) return;

#ifndef SIMULATOR
  const uint32_t r1 = esp_random();
  const uint32_t r2 = esp_random();
#else
  const uint32_t r1 = static_cast<uint32_t>(rand());
  const uint32_t r2 = static_cast<uint32_t>(rand());
#endif

  snprintf(outSalt, maxLen, "%08lx%08lx", static_cast<unsigned long>(r1), static_cast<unsigned long>(r2));
}

void PinStorage::computeHash(const char* salt, const char* pin, char* outHash, const size_t maxLen) {
  if (salt == nullptr || pin == nullptr || outHash == nullptr || maxLen <= HASH_LEN) return;

  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);  // 0 = SHA-256

  mbedtls_sha256_update(&ctx, reinterpret_cast<const unsigned char*>(salt), strlen(salt));
  mbedtls_sha256_update(&ctx, reinterpret_cast<const unsigned char*>(pin), strlen(pin));

  unsigned char digest[32];
  mbedtls_sha256_finish(&ctx, digest);
  mbedtls_sha256_free(&ctx);

  for (size_t i = 0; i < 32; ++i) {
    snprintf(outHash + (i * 2), 3, "%02x", digest[i]);
  }
  outHash[HASH_LEN] = '\0';
}
