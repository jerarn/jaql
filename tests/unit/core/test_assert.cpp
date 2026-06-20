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

  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "debug check");
  EXPECT_FALSE(std::string{recorder.last.location.file_name()}.empty());
  EXPECT_GT(recorder.last.location.line(), 0U);
}

TEST(Assert, ThrowingHandler_FailedAssert_ThrowsAssertionViolation) {
  ViolationHandlerGuard handler_guard{throwing_handler};

  try {
    JAQL_ASSERT(false, "should throw");
    FAIL() << "JAQL_ASSERT(false) did not throw";
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

  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "precondition");
}

TEST(Assert, Ensures_FailingCondition_InvokesHandler) {
  ViolationHandlerGuard handler_guard{recording_handler};
  ViolationRecorder recorder{};
  RecorderGuard recorder_guard{recorder};

  JAQL_ENSURES(false, "postcondition");

  ASSERT_TRUE(recorder.invoked);
  EXPECT_EQ(recorder.last.expression, "false");
  EXPECT_EQ(recorder.last.message, "postcondition");
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
