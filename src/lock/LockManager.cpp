#include "LockManager.h"

#include "CrossPointSettings.h"
#include "Logging.h"
#include "PinStorage.h"

void LockManager::init() {
  PinStorage::checkAndApplySdRecovery();

  if (PinStorage::isLockEnabled()) {
    if (SETTINGS.lockOnBoot != 0) {
      state_ = LockState::Locked;
      trigger_ = LockTrigger::Boot;
      LOG_INF("LOCK", "Device locked on boot");
    } else {
      state_ = LockState::Unlocked;
      trigger_ = LockTrigger::None;
      LOG_INF("LOCK", "Device lock enabled, but lock on boot is disabled");
    }
  } else {
    state_ = LockState::Disabled;
    trigger_ = LockTrigger::None;
  }
}

bool LockManager::shouldLockOnBoot() const {
  return PinStorage::isLockEnabled() && (SETTINGS.lockOnBoot != 0);
}

bool LockManager::shouldLockOnSleep() const {
  return PinStorage::isLockEnabled() && (SETTINGS.lockOnSleep != 0);
}

bool LockManager::shouldLockOnSleepWake(const bool fromTimeout, const uint32_t elapsedSleepSeconds, const bool forcePinLock) const {
  if (!PinStorage::isLockEnabled()) {
    return false;
  }
  if (SETTINGS.lockOnSleep == 0) {
    return false;
  }
  // If forced lock from long press: require PIN immediately on wake
  if (forcePinLock) {
    LOG_INF("LOCK", "Forced lock from long press: requiring PIN immediately on wake");
    return true;
  }
  // For basic short-press sleep and auto-sleep: only require PIN if inactivity timeout elapsed
  const uint32_t pinTimeoutSec = static_cast<uint32_t>(SETTINGS.pinTimeoutMinutes) * 60;
  if (pinTimeoutSec == 0) {
    LOG_INF("LOCK", "PIN timeout is 0 (Immediately): requiring PIN on wake");
    return true;
  }
  const bool expired = (elapsedSleepSeconds >= pinTimeoutSec);
  LOG_INF("LOCK", "Sleep wake: elapsed=%lu s, threshold=%lu s (fromTimeout=%d) -> %s",
          static_cast<unsigned long>(elapsedSleepSeconds),
          static_cast<unsigned long>(pinTimeoutSec),
          fromTimeout ? 1 : 0,
          expired ? "LOCKED" : "UNLOCKED (grace period)");
  return expired;
}

bool LockManager::shouldLockOnQuickLock() const {
  return PinStorage::isLockEnabled() && (SETTINGS.lockOnQuickLock != 0);
}

void LockManager::lock(const LockTrigger trigger) {
  if (PinStorage::isLockEnabled()) {
    state_ = LockState::Locked;
    trigger_ = trigger;
    LOG_INF("LOCK", "Device locked (trigger: %d)", static_cast<int>(trigger));
  }
}

void LockManager::unlock() {
  if (state_ == LockState::Locked) {
    state_ = LockState::Unlocked;
    trigger_ = LockTrigger::None;
    LOG_INF("LOCK", "Device unlocked");
  }
}
