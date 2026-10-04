#pragma once

#include <cstdint>

enum class LockState : uint8_t {
  Disabled,
  Locked,
  Unlocked
};

enum class LockTrigger : uint8_t {
  None,
  Boot,
  SleepWake,
  QuickLock,
  Manual
};

class LockManager {
 public:
  static LockManager& getInstance() {
    static LockManager instance;
    return instance;
  }

  void init();

  LockState getState() const { return state_; }
  bool isLocked() const { return state_ == LockState::Locked; }
  LockTrigger getTrigger() const { return trigger_; }

  bool shouldLockOnBoot() const;
  bool shouldLockOnSleep() const;
  bool shouldLockOnQuickLock() const;

  void lock(LockTrigger trigger);
  void unlock();

 private:
  LockManager() = default;
  LockState state_ = LockState::Disabled;
  LockTrigger trigger_ = LockTrigger::None;
};
