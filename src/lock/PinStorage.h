#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class PinStorage {
 public:
  static constexpr size_t MIN_PIN_LEN = 4;
  static constexpr size_t MAX_PIN_LEN = 6;
  static constexpr size_t SALT_LEN = 16;
  static constexpr size_t HASH_LEN = 64;

  // Checks whether a valid PIN has been configured
  static bool hasPin();

  // Returns true if device lock is enabled and a PIN exists
  static bool isLockEnabled();

  // Verifies the entered PIN against the stored hash and salt
  static bool verifyPin(const char* pin);

  // Sets a new PIN: generates a salt, computes SHA-256 hash, and saves settings
  static bool setPin(const char* pin);

  // Clears the PIN and disables device lock
  static void clearPin();

  // Checks if a recovery reset file exists on the SD card (/.crosspoint/pin_reset.txt or /pin_reset.txt)
  // If found, resets the PIN and deletes/renames the file so the user is never locked out.
  static bool checkAndApplySdRecovery();

  // Security question recovery
  enum SecurityQuestion : uint8_t { GreatestLove = 0, FirstPet = 1, FavoriteCountry = 2, QUESTION_COUNT = 3 };

  static bool hasSecurityQuestion();
  static uint8_t getSecurityQuestion();
  static bool setSecurityQuestion(uint8_t questionIdx, const char* answer);
  static bool verifySecurityAnswer(const char* answer);
  static std::string normalizeAnswer(const char* answer);

  // Cryptographic utilities
  static void generateSalt(char* outSalt, size_t maxLen);
  static void computeHash(const char* salt, const char* pin, char* outHash, size_t maxLen);
};
