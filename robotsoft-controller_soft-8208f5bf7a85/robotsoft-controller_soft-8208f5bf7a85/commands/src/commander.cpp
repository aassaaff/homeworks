// #include "commands/commander.h"
#include "commands/commander.h"

namespace commands {

CommandRunner::CommandRunner() {
    m_commands_to_exec.reserve(256);
    m_running_commands.reserve(256);
}

CommandRunner::~CommandRunner() {

}

void CommandRunner::executeCommands() {
    auto iter = m_commands_to_exec.begin();

    //infra::Logger::log().info("CommandRunner::execute is executing commands");
    bool cont = true;
    while ((iter != m_commands_to_exec.end()) && cont) {
        bool confirmed = (*iter)->isConfirmed();
        infra::Logger::log().info("CommandRunner::execute is executing command {} confirmed={}", (*iter)->getName(), confirmed);
        /* TODO: Remove the "if" below once we get rid of the confirmation option */
        if ((!confirmed) && ((!confirmCommand(*(*iter))) || (!confirmCommandMissionInfo(*(*iter))))) {
            (*iter)->drop();
            infra::Logger::log().info("CommandRunner::execute {} is unconfirmed command. Skipping", (*iter)->getName());
            iter = m_commands_to_exec.erase(iter);
            continue;
        }
        if (!((*iter)->activatePreExecOp())) {
            (*iter)->drop();
            infra::Logger::log().info("CommandRunner::execute {} pre exec did not confirm to continue with the command", (*iter)->getName());
            iter = m_commands_to_exec.erase(iter);
            continue;
        }
        cont = (*iter)->execute();
        infra::Logger::log().info("CommandRunner::execute {} on command {}", cont ? "continue" : "hold", (*iter)->getName());
        m_running_commands.push_back(*iter);
        iter = m_commands_to_exec.erase(iter);
    }
    if ((m_commands_to_exec.empty()) && (m_running_commands.empty())) {
        executeEndedImp(true);
    }
    infra::Logger::log().info("CommandRunner::execute cycle ended");
}

void CommandRunner::confirmAll() {
    infra::Logger::log().info("CommandRunner::confirmAll confirming all commands start");
    for (auto &cmd : m_commands_to_exec) {
        cmd->confirm();
        infra::Logger::log().info("CommandRunner::confirmAll {} has been confirmed", cmd->getName());
    }
    infra::Logger::log().info("CommandRunner::confirmAll confirming all commands end");
}

void CommandRunner::startedCommands(MissionCommand &command) {
    infra::Logger::log().info("CommandRunner::started {}", command.getName());
}

void CommandRunner::progressCommands(MissionCommand &command, bool print) {
    if (print) {
        infra::Logger::log().debug("CommandRunner::progress ({})", command.getName());
    }
    progressUpdatedImp();
}

bool CommandRunner::completedCommands(MissionCommand &command, bool success) {
    if ((success = command.activatePostExecOp(success))) {
        infra::Logger::log().info("CommandRunner::completed. ok ({})", command.getName());
    } else {
        infra::Logger::log().info("CommandRunner::completed. failed ({})", command.getName());
        if (!command.canContinueAfterFailure()) {
            failCommand(command);
        } else {
            infra::Logger::log().info("CommandRunner::command failed but allow next commands to be executed");
        }
    }
    auto iter = m_running_commands.begin();
    while (iter != m_running_commands.end()) {
        if (*iter == &command) {
            m_running_commands.erase(iter);
            break;
        }
        iter++;
    }
    return success;
}

void CommandRunner::groupCompleted(MissionCommand &command, bool success) {
    infra::Logger::log().info("CommandRunner::groupCompleted. success={} ({})", success, command.getName());
    bool continueAfterFailure = command.canContinueAfterFailure();
    command.drop();
    m_running_commands.clear();
    if (success || continueAfterFailure) {
        if (!success) {
            infra::Logger::log().warn("CommandRunner::groupCompleted - command failed without stopping mission");
            // infra::ErrorManager::instance().setError(infra::ErrorManager::instance().m_error_data.codes.rmanager.ContinueAfterFailure);
        }
        executeCommands();
    } else {
        executeEndedImp(false);
    }
}

void CommandRunner::cancelCommands() {
    if (m_commands_to_exec.empty() && m_running_commands.empty()) {
        infra::Logger::log().info("CommandRunner::cancel: Nothing left to cancel.");
        executeEndedImp(false);
        return;
    }

    auto iter = m_running_commands.begin();
    while (iter != m_running_commands.end()) {
        infra::Logger::log().info("CommandRunner::cancel is canceling command {}", (*iter)->getName());
        if ((*iter)->isNoWayBack()) {
            infra::Logger::log().info("CommandRunner::cancel stopped because we reach the non return point in {}", (*iter)->getName());
            return;
        }
        (*iter)->activatePreCancelOp();
        (*iter)->cancel();
        iter = m_running_commands.erase(iter);
        infra::Logger::log().info("CommandRunner::cancel is canceling command ended");
    }

    if (!m_commands_to_exec.empty()) {
        infra::Logger::log().info("CommandRunner::cancel: clearing non started commands.");
        for (auto cmd : m_commands_to_exec) {
            cmd->drop();
        }
        m_commands_to_exec.clear();
    }
}

void CommandRunner::addCommand(MissionCommand &command) {
    m_commands_to_exec.push_back(&command);
}

void CommandRunner::addCommandFirst(MissionCommand &command) {
    m_commands_to_exec.insert(m_commands_to_exec.begin(), &command);
}

void CommandRunner::addCommandAfter(MissionCommand &after, MissionCommand &command) {
    auto iter = m_commands_to_exec.begin();
    for (; iter != m_commands_to_exec.end(); iter++) {
        if (*iter == &after) {
            iter++;
            if (iter == m_commands_to_exec.end()) {
                m_commands_to_exec.push_back(&command);
            } else if ((*iter != &command)) {
                // Do not add the same command twice
                m_commands_to_exec.insert(iter, &command);
            }
            break;
        }
    }
}

void CommandRunner::addCommandBefore(MissionCommand &before, MissionCommand &command) {
    auto iter = m_commands_to_exec.begin();
    for (; iter != m_commands_to_exec.end(); iter++) {
        if (*iter == &command) {
            // Do not add the same command twice
            break;
        }
        if (*iter == &before) {
            m_commands_to_exec.insert(iter, &command);
            break;
        }
    }
}

void CommandRunner::addUnconfirmedCommand(MissionCommand &command) {
    command.setUnconfirmed();
    addCommand(command);
}

void CommandRunner::addNoWayBackCommand(MissionCommand &command) {
    command.setNoWayBack();
    addCommand(command);
}

bool CommandRunner::confirmCommand(MissionCommand &command) {
    return false;
}

bool CommandRunner::confirmCommandMissionInfo(MissionCommand &command) {
    return false;
}

void CommandRunner::failCommand(MissionCommand &command) {
    // Do not start pending commands
    for (auto cmd : m_commands_to_exec) {
        cmd->drop();
    }
    m_commands_to_exec.clear();

    infra::Logger::log().info("CommandRunner::failCommand started");

    commandFailedImp(command);

    auto iter = m_running_commands.begin();
    while (iter != m_running_commands.end()) {
        if (*iter != &command) {
            // Cancel running commands
            (*iter)->cancel();
        }
        iter = m_running_commands.erase(iter);
    }

    infra::Logger::log().info("CommandRunner::failCommand ended");
}

MissionCommand::MissionCommand() :
    m_cmd_runner(nullptr), m_done(false), m_success(false), m_name_suffix(""), m_no_way_back(false), m_confirmed(true),
    m_canceled(false), m_continue_after_failure(false), m_pre_exec_op(nullptr), m_pre_exec_obj(nullptr),
    m_pre_exec_priv_obj(nullptr), m_post_exec_op(nullptr), m_post_exec_obj(nullptr), m_exec_obj(nullptr),
    m_pre_cancel_op(nullptr), m_pre_cancel_obj(nullptr), m_print_in_progress(true) {

}

MissionCommand::~MissionCommand() {
}

bool MissionCommand::init(CommandRunner &listener, std::string name_suffix) {
    m_cmd_runner = &listener;
    m_name_suffix = name_suffix;
    return true;
}

bool MissionCommand::execute() {
    m_done = false;
    return executeImp();
}

void MissionCommand::cancel() {
    m_canceled = true;
    cancelImp();
}

bool MissionCommand::gotCancel() {
    return m_canceled;
}

bool MissionCommand::isExecuting() {
    return !isDone();
}

bool MissionCommand::isDone() {
    return m_done;
}

bool MissionCommand::isSuccessful() {
    return m_success;
}

void MissionCommand::setNoWayBack() {
    m_no_way_back = true;
}

void MissionCommand::setUnconfirmed() {
    m_confirmed = false;
}

void MissionCommand::setConfirmed() {
    m_confirmed = true;
}

bool MissionCommand::isConfirmed() {
    return m_confirmed;
}

void MissionCommand::confirm() {
    m_confirmed = true;
}

bool MissionCommand::canContinueAfterFailure() {
    return m_continue_after_failure;
}

void MissionCommand::allowContinueAfterFailure() {
    m_continue_after_failure = true;
}

void MissionCommand::drop() {
    m_confirmed = true;
    m_no_way_back = false;
    m_canceled = false;
    m_continue_after_failure = false;
    if (!containingSubCommands()) {
        m_pre_exec_op = nullptr;
        m_pre_exec_obj = nullptr;
        m_pre_exec_priv_obj = nullptr;
        m_post_exec_op = nullptr;
        m_post_exec_obj = nullptr;
        m_exec_obj = nullptr;
        m_pre_cancel_op = nullptr;
        m_pre_cancel_obj = nullptr;
    }
    dropImp();
}

bool MissionCommand::isNoWayBack() {
    return m_no_way_back;
}

const std::string MissionCommand::getName() const {
    return m_name_suffix == "" ? getNameImp() : getNameImp() + "__" + m_name_suffix;
}

void MissionCommand::setPreExecOp(Object object, bool (*pre_exec_op)(MissionCommand &)) {
    m_pre_exec_op = pre_exec_op;
    m_pre_exec_obj = object;
}

void MissionCommand::setPostExecOp(Object object, bool (*post_exec_op)(MissionCommand &, bool)) {
    m_post_exec_op = post_exec_op;
    m_post_exec_obj = object;
}

void MissionCommand::setPreCancelOp(Object object, void (*pre_cancel_op)(MissionCommand &)) {
    m_pre_cancel_op = pre_cancel_op;
    m_pre_cancel_obj = object;
}

bool MissionCommand::activatePreExecOp() {
    m_exec_obj = m_pre_exec_obj;
    bool res = m_pre_exec_op ? m_pre_exec_op(*this) : true;
    m_exec_obj = m_pre_exec_priv_obj;
    activatePreExecOpImp();
    return res;
}

void MissionCommand::activatePreCancelOp() {
    m_exec_obj = m_pre_cancel_obj;
    if (m_pre_cancel_op) {
        m_pre_cancel_op(*this);
    }
}

bool MissionCommand::activatePostExecOp(bool success) {
    if (containingSubCommands()) {
        return success;
    }
    m_exec_obj = m_post_exec_obj;
    return m_post_exec_op ? m_post_exec_op(*this, success) : success;
}

void MissionCommand::addAfterCommand(MissionCommand &after, MissionCommand &command) {
    if (m_cmd_runner) {
        m_cmd_runner->addCommandAfter(after, command);
    }
}

void MissionCommand::started() {
    if (m_cmd_runner) {
        m_cmd_runner->startedCommands(*this);
    }
}

void MissionCommand::progress() {
    if (m_cmd_runner) {
        m_cmd_runner->progressCommands(*this, m_print_in_progress);
        m_print_in_progress = false;
    }
}

void MissionCommand::completed(bool success) {
    success = m_canceled ? false : success;
    m_no_way_back = false;
    m_confirmed = true;
    m_done = true;
    m_success = success;
    m_canceled = false;
    m_print_in_progress = true;
    if (m_cmd_runner) {
        success = m_cmd_runner->completedCommands(*this, success);
    }
    if (groupCompletedCount()) {
        m_cmd_runner->groupCompleted(*this, success);
    } else {
        drop();
    }
}

void MissionCommand::setPrivateActivatePreExecObj(Object obj) {
    m_pre_exec_priv_obj = obj;
}

void MissionCommand::activatePreExecOpImp() {

}

bool MissionCommand::containingSubCommands() {
    return false;
}

MissionBarrier::MissionBarrier() : MissionCommand(), m_running(0), m_ended(0), m_started_exe(false), m_group_success(true) {

}

MissionBarrier::~MissionBarrier() {

}

void MissionBarrier::notifyEnded(bool success, MissionCommand &command) {
    infra::Logger::log().info(
        "MissionBarrier::notifyEnded - barrier notified ended from command {}. m_ended={}, m_running={}, m_started_exe={}, m_group_success={}",
        command.getName(), m_ended, m_running, m_started_exe, m_group_success);
    m_group_success = m_group_success ? success : m_group_success;
    progressImp();
}

void MissionBarrier::addBarrieredMission() {
    infra::Logger::log().info("MissionBarrier::addBarrieredMission added to barrier {}", getNameImp());
    m_running++;
}

void MissionBarrier::dropImp() {
    infra::Logger::log().info("MissionBarrier::dropImp Barrier {} dropped", getNameImp());
    m_running = 0;
    m_ended = 0;
    m_started_exe = false;
    m_group_success = true;
}

bool MissionBarrier::executeImp() {
    m_group_success = true;
    startedImp();
    for (uint32_t i = 0; i < m_ended; i++) {
        progress();
    }
    if (!m_running) {
        completedImp(m_group_success);
    }
    // Barrier is always  handled via completed context.
    return false;
}

void MissionBarrier::startedImp() {
    m_started_exe = true;
    started();
}

void MissionBarrier::progressImp() {
    m_ended++;
    m_running--;
    if (!m_started_exe) {
        // If we did not start to execute, do not call
        // callbacks. We will call them when execute will be called
        return;
    }
    progress();
    if (!m_running) {
        completedImp(m_group_success);
    }
}

void MissionBarrier::completedImp(bool success) {
    m_ended = 0;
    m_running = 0;
    m_started_exe = false;
    completed(success);
}

void MissionBarrier::cancelImp() {

}

bool MissionBarrier::groupCompletedCount() {
    return !m_running;
}

const std::string MissionBarrier::getNameImp() const {
    return "Barrier";
}

BlockableCommand::BlockableCommand() : MissionCommand(), m_barrier(nullptr) {

}

BlockableCommand::~BlockableCommand() {

}

bool BlockableCommand::init(CommandRunner &listener, std::string name_suffix) {
    return MissionCommand::init(listener, name_suffix);
}

void BlockableCommand::setBarrier(MissionBarrier &barrier) {
    m_barrier = &barrier;
    m_barrier->addBarrieredMission();
}

void BlockableCommand::dropImp() {
    m_barrier = nullptr;
}

bool BlockableCommand::groupCompletedCount() {
    if (m_barrier) {
        m_barrier->notifyEnded(isSuccessful(), *this);
        m_barrier = nullptr;
        return false;
    }
    return true;
}

bool BlockableCommand::shouldContinue() {
    return m_barrier ? true : false;
}

bool BlockableCommandCommandRunner::initBlockableCommandCommandRunner(commands::CommandRunner &listener, std::string name_suffix) {
    return BlockableCommand::init(listener, std::move(name_suffix));
}

void BlockableCommandCommandRunner::startedImp() {
    started();
}

void BlockableCommandCommandRunner::progressImp() {
    progress();
}

void BlockableCommandCommandRunner::completedImp(bool success) {
    completed(success);
}

void BlockableCommandCommandRunner::cancelImp() {
    infra::Logger::log().info("BlockableCommandCommandRunner::cancelImp - cancelling");
    cancelCommands();
}

void BlockableCommandCommandRunner::commandFailedImp(commands::MissionCommand &command) {
    completedImp(false);
}

void BlockableCommandCommandRunner::executeEndedImp(bool success) {
    infra::Logger::log().info("BlockableCommandCommandRunner::executeEndedImp - result {}", success ? "success" : "failure");
    completedImp(success);
}

void BlockableCommandCommandRunner::progressUpdatedImp() {

}

LoopCommand::LoopCommand() : m_itr_count(0), m_have_more_iters(false), m_predicate(nullptr), m_last_command_added(this),
                             m_loop_buddy(nullptr) {

}

LoopCommand::~LoopCommand() {

}

bool LoopCommand::init(CommandRunner &listener, std::string name_suffix) {
    return BlockableCommand::init(listener, name_suffix);
}

void LoopCommand::setLoopBody(uint32_t count, Object object, void(*loop_boody)(MissionCommand &)) {
    m_loop_buddy = loop_boody;
    setPrivateActivatePreExecObj(object);
    m_itr_count = count;
}

void LoopCommand::activatePreExecOpImp() {
    if (m_itr_count) {
        m_have_more_iters = true;
        infra::Logger::log().info("LoopCommand::activatePreExecOpImp - {} m_itr_count={}", getName(), m_itr_count);
        m_itr_count--;
    } else {
        m_have_more_iters = false;
        return;
    }
    if (!m_predicate) {
        m_loop_buddy(*this);
        infra::Logger::log().info("LoopCommand::activatePreExecOpImp - {} no predicate", getName());
        addAfterCommand(*m_last_command_added, *this);
        m_last_command_added = this;
        m_have_more_iters = true;
    } else if (m_predicate(*this)) {
        infra::Logger::log().info("LoopCommand::activatePreExecOpImp - {} predicate break, m_itr_count={}", getName(), m_itr_count);
        m_itr_count = 0;
        m_have_more_iters = false;
        return;
    } else {
        m_loop_buddy(*this);
        infra::Logger::log().info("LoopCommand::activatePreExecOpImp - {} no predicate break. Adding me after {}",
                                  getName(),
                                  m_last_command_added->getName());
        addAfterCommand(*m_last_command_added, *this);
        m_last_command_added = this;
        m_have_more_iters = true;
    }
}

bool LoopCommand::containingSubCommands() {
    infra::Logger::log().info("LoopCommand::containingSubCommands - m_itr_count={}, m_last_iter={}", m_itr_count, m_have_more_iters);
    return m_have_more_iters;
}

void LoopCommand::setBreakCondition(bool(*predicate)(MissionCommand &)) {
    m_predicate = predicate;
}

void LoopCommand::addLoopCommand(MissionCommand &command) {
    infra::Logger::log().info("LoopCommand::addLoopCommand - adding loop command {} after {}", command.getName(), m_last_command_added->getName());
    addAfterCommand(*m_last_command_added, command);
    m_last_command_added = &command;
}

void LoopCommand::dropImp() {
    BlockableCommand::dropImp();
}

bool LoopCommand::executeImp() {
    m_timer.setTimer(ITERATION_WAIT_DURATION, *this);
    return shouldContinue();
}

void LoopCommand::startedImp() {
    started();
}

void LoopCommand::progressImp() {
    progress();
}

void LoopCommand::cancelImp() {

}

void LoopCommand::completedImp(bool success) {
    completed(success);
}

const std::string LoopCommand::getNameImp() const {
    return "LoopCommand";
}

void LoopCommand::expired() {
    infra::Logger::log().info("LoopCommand::expired Success command triggered.");
    completedImp(true);
}

const double LoopCommand::ITERATION_WAIT_DURATION = 0.1;

WaitCommand::WaitCommand() : BlockableCommand(), m_duration(0), m_retries(0), m_op_count(0) {

}

WaitCommand::~WaitCommand() {

}

bool WaitCommand::init(CommandRunner &listener, std::string name_suffix) {
    return BlockableCommand::init(listener, name_suffix);
}

void WaitCommand::setDuration(double duration, uint32_t retries) {
    m_duration = duration;
    m_retries = retries;
}

void WaitCommand::stopTimer() {
    infra::Logger::log().info("WaitCommand::stopTimer - start");
    if (isExecuting()) {
        infra::Logger::log().info("WaitCommand::stopTimer - stopping timer & completing");
        m_timer.stopTimer();
        completedImp(true);
    }
}

void WaitCommand::dropImp() {
    BlockableCommand::dropImp();
    m_retries = 0;
    m_duration = 0.0;
    m_op_count = 0;
}

bool WaitCommand::executeImp() {
    m_op_count = 0;
    executeOperation(m_op_count);
    m_timer.setTimer(m_duration ? m_duration : DEFAULT_WAIT_DURATION, *this);
    return shouldContinue();
}

void WaitCommand::startedImp() {
    started();
}

void WaitCommand::progressImp() {
    progress();
}

void WaitCommand::cancelImp() {

}

void WaitCommand::completedImp(bool success) {
    completed(success);
}

const std::string WaitCommand::getNameImp() const {
    return "WaitCommand";
}

void WaitCommand::executeOperation(uint32_t count) {

}

bool WaitCommand::getOperationResult() {
    return true;
}

void WaitCommand::expired() {
    if (getOperationResult()) {
        infra::Logger::log().info("WaitCommand::expired - Wait command {} succeeded", getName());
        // Result OK. completed ok.
        m_retries = 0;
        m_duration = 0.0;
        m_op_count = 0;
        completedImp(true);
    } else if (m_retries) {
        infra::Logger::log().info("WaitCommand::expired - Wait command {} retry {}", getName(), m_retries);
        // Result not OK but still have retries.
        executeOperation(m_op_count);
        m_timer.setTimer(m_duration, *this);
        m_retries--;
        m_op_count++;
    } else {
        infra::Logger::log().info("WaitCommand::expired - Wait command {} failed", getName());
        // Result not OK and no more retries.
        m_op_count = 0;
        completedImp(false);
    }
}

const double WaitCommand::DEFAULT_WAIT_DURATION = 0.1;

FailCommand::FailCommand() : WaitCommand(), m_error_code("") {

}

FailCommand::~FailCommand() {

}

void FailCommand::setError(const std::string &errorCode) {
    m_error_code = errorCode;
}

void FailCommand::completedImp(bool success) {
    if (!success && !m_error_code.empty()) {
        // infra::ErrorManager::instance().setError(m_error_code);
    }
    completed(success);
}

const std::string FailCommand::getNameImp() const {
    return "AbortAllCommand";
}

bool FailCommand::getOperationResult() {
    return false;
}

NoopCommand::NoopCommand() : WaitCommand() {

}

NoopCommand::~NoopCommand() {

}

const std::string NoopCommand::getNameImp() const {
    return "NopCommand";
}

MarkerCommand::MarkerCommand() : WaitCommand() {

}

MarkerCommand::~MarkerCommand() {

}

const std::string MarkerCommand::getNameImp() const {
    return "MarkerCommand";
}

ImageSaverCommand::ImageSaverCommand() : MissionCommand() {
    // ros::NodeHandle nh;
    // m_request_image_save_pub = nh.advertise<caja_msgs::SaveImage>("/ro_request_image_save", 0);
}

void ImageSaverCommand::dropImp() {
    m_camera_enum = 0;
}

const std::string ImageSaverCommand::getNameImp() const {
    return "ImageSaver";
}

bool ImageSaverCommand::executeImp() {
    // caja_msgs::SaveImage saveImageMsg;
    // saveImageMsg.picture.module = m_camera_enum;
    // m_request_image_save_pub.publish(saveImageMsg);
    return true;
}

void ImageSaverCommand::startedImp() {
    started();
}

void ImageSaverCommand::progressImp() {
    progress();
}

void ImageSaverCommand::completedImp(bool success) {
    m_camera_enum = 0;
    completed(success);
}

void ImageSaverCommand::cancelImp() {
    m_camera_enum = 0;
}

bool ImageSaverCommand::groupCompletedCount() {
    return true;
}

void ImageSaverCommand::setCameraEnumString(int cameraEnum) {
    m_camera_enum = cameraEnum;
}

ImageSaverCommand::~ImageSaverCommand() {

}

}; // namespace infra
