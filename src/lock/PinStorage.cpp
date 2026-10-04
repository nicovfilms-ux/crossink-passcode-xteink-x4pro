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
