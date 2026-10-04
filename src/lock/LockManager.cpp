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
