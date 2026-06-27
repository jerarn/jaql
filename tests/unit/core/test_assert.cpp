#include <jaql/core/assert.hpp>

#include <gtest/gtest.h>

#include <string>

using jaql::core::abort_handler;
using jaql::core::AssertionViolation;
using jaql::core::get_violation_handler;
using jaql::core::set_violation_handler;
using jaql::core::throwing_handler;
using jaql::core::ViolationHandler;
using jaql::core::ViolationInfo;

// Mirror the activation conditions of the assertion macros in <jaql/core/assert.hpp>
// so the expectations track the build configuration. JAQL_ASSERT is stripped under
// NDEBUG; JAQL_EXPECTS/JAQL_ENSURES additionally remain active when contracts are enabled.
#if !defined(NDEBUG)
#define JAQL_TEST_ASSERT_ACTIVE 1
#else
#define JAQL_TEST_ASSERT_ACTIVE 0
#endif

#if !defined(NDEBUG) || defined(JAQL_ENABLE_CONTRACTS)
#define JAQL_TEST_CONTRACTS_ACTIVE 1
#else
#define JAQL_TEST_CONTRACTS_ACTIVE 0
#endif

namespace {

struct ViolationRecorder {
  bool invoked = false;
  ViolationInfo last{};
};

ViolationRecorder* g_recorder = nullptr;

void recording_handler(const ViolationInfo& info) {
  if (g_recorder != nullptr) {
    g_recorder->invoked = true;
    g_recorder->last = info;
  }
}

struct ViolationHandlerGuard {
  explicit ViolationHandlerGuard(ViolationHandler handler) : previous_{get_violation_handler()} {
    set_violation_handler(handler);
  }

  ~ViolationHandlerGuard() { set_violation_handler(previous_); }

  ViolationHandlerGuard(const ViolationHandlerGuard&) = delete;
  auto operator=(const ViolationHandlerGuard&) -> ViolationHandlerGuard& = delete;
  ViolationHandlerGuard(ViolationHandlerGuard&&) = delete;
  auto operator=(ViolationHandlerGuard&&) -> ViolationHandlerGuard& = delete;

 private:
  ViolationHandler previous_;
};

struct RecorderGuard {
  explicit RecorderGuard(ViolationRecorder& recorder) : recorder_{recorder} {
    g_recorder = &recorder_;
  }

  ~RecorderGuard() {
    if (g_recorder == &recorder_) {
      g_recorder = nullptr;
    }
  }

  RecorderGuard(const RecorderGuard&) = delete;
  auto operator=(const RecorderGuard&) -> RecorderGuard& = delete;
  RecorderGuard(RecorderGuard&&) = delete;
  auto operator=(RecorderGuard&&) -> RecorderGuard& = delete;

 private:
  ViolationRecorder& recorder_;
};

}  // namespace

TEST(Assert, PassingCondition_DoesNotInvokeHandler) {
  ViolationHandlerGuard handler_guard{recording_handler};
  ViolationRecorder recorder{};
  RecorderGuard recorder_guard{recorder};

  JAQL_ASSERT(true);

  EXPECT_FALSE(recorder.invoked);
}

TEST(Assert, FailingCondition_InvokesHandler) {
  ViolationHandlerGuard handler_guard{recording_handler};
  ViolationRecorder recorder{};
  RecorderGuard recorder_guard{recorder};

  JAQL_ASSERT(false, "debug check");

#if JAQL_TEST_ASSERT_ACTIVE
  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "debug check");
  EXPECT_FALSE(std::string{recorder.last.location.file_name()}.empty());
  EXPECT_GT(recorder.last.location.line(), 0U);
#else
  EXPECT_FALSE(recorder.invoked);
#endif
}

TEST(Assert, ThrowingHandler_FailedAssert_ThrowsAssertionViolation) {
  ViolationHandlerGuard handler_guard{throwing_handler};

  try {
    JAQL_ASSERT(false, "should throw");
#if JAQL_TEST_ASSERT_ACTIVE
    FAIL() << "JAQL_ASSERT(false) did not throw";
#endif
  } catch (const AssertionViolation& error) {
    EXPECT_EQ(error.info().expression, "false");
    EXPECT_EQ(error.info().message, "should throw");
    EXPECT_NE(std::string{error.what()}.find("false"), std::string::npos);
    EXPECT_NE(std::string{error.what()}.find("should throw"), std::string::npos);
  }
}

TEST(Assert, Expects_FailingCondition_InvokesHandler) {
  ViolationHandlerGuard handler_guard{recording_handler};
  ViolationRecorder recorder{};
  RecorderGuard recorder_guard{recorder};

  JAQL_EXPECTS(false, "precondition");

#if JAQL_TEST_CONTRACTS_ACTIVE
  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "precondition");
#else
  EXPECT_FALSE(recorder.invoked);
#endif
}

TEST(Assert, Ensures_FailingCondition_InvokesHandler) {
  ViolationHandlerGuard handler_guard{recording_handler};
  ViolationRecorder recorder{};
  RecorderGuard recorder_guard{recorder};

  JAQL_ENSURES(false, "postcondition");

#if JAQL_TEST_CONTRACTS_ACTIVE
  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "postcondition");
#else
  EXPECT_FALSE(recorder.invoked);
#endif
}

TEST(Assert, ViolationHandlerGuard_RestoresPreviousHandler) {
  const ViolationHandler original = get_violation_handler();
  {
    ViolationHandlerGuard handler_guard{recording_handler};
    EXPECT_EQ(get_violation_handler(), recording_handler);
  }
  EXPECT_EQ(get_violation_handler(), original);
}

TEST(Assert, DefaultHandler_IsAbortHandler) {
  ViolationHandlerGuard handler_guard{abort_handler};
  EXPECT_EQ(get_violation_handler(), abort_handler);
}
