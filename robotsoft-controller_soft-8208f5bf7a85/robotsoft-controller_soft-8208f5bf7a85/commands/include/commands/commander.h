#ifndef __COMMANDER_H__
#define __COMMANDER_H__

// #include <string>
// #include <vector>
#include "infra/timer.h"
#include "infra/logger.h"

//#include <caja_msgs/SaveImage.h>

namespace commands {

class MissionCommand;

class CommandRunner {
 public:
    CommandRunner();
    virtual ~CommandRunner();
    void executeCommands();
    void confirmAll();
    void startedCommands(MissionCommand &command);
    void progressCommands(MissionCommand &command, bool print = true);
    bool completedCommands(MissionCommand &command, bool success);
    void groupCompleted(MissionCommand &command, bool sucess);
    void cancelCommands();
    void addCommand(MissionCommand &command);
    void addCommandFirst(MissionCommand &command);
    void addCommandAfter(MissionCommand &after, MissionCommand &command);
    void addCommandBefore(MissionCommand &before, MissionCommand &command);
    void addUnconfirmedCommand(MissionCommand &command);
    void addNoWayBackCommand(MissionCommand &command);
    inline int getCommandsToExecCount() const { return m_commands_to_exec.size(); }
    inline const std::vector<MissionCommand *> getCommandsToExec() const { return m_commands_to_exec; }
 protected:
    virtual void commandFailedImp(MissionCommand &command) = 0;
    virtual void executeEndedImp(bool success) = 0;
    virtual void progressUpdatedImp() = 0;
    virtual bool confirmCommand(MissionCommand &command);
    virtual bool confirmCommandMissionInfo(MissionCommand &command);

 private:
    void failCommand(MissionCommand &command);
 private:
    std::vector<MissionCommand *> m_commands_to_exec;
    std::vector<MissionCommand *> m_running_commands;
};

typedef void *Object;

class MissionCommand {
 public:
    MissionCommand();
    virtual ~MissionCommand();

    bool init(CommandRunner &listener, std::string name_suffix);
    bool execute();
    void cancel();
    bool gotCancel();
    bool isExecuting();
    bool isDone();
    bool isSuccessful();
    void setNoWayBack();
    void setUnconfirmed();
    void setConfirmed();
    bool isConfirmed();
    void confirm();
    bool canContinueAfterFailure();
    void allowContinueAfterFailure();
    void drop();
    bool isNoWayBack();
    const std::string getName() const;
    void setPreExecOp(Object object, bool (*pre_exec_op)(MissionCommand &));
    void setPostExecOp(Object object, bool (*post_exec_op)(MissionCommand &, bool success));
    void setPreCancelOp(Object object, void (*pre_cancel_op)(MissionCommand &));
    bool activatePreExecOp();
    void activatePreCancelOp();
    bool activatePostExecOp(bool success);
    void addAfterCommand(MissionCommand &after, MissionCommand &command);

    template<typename T>
    T &getRunnerObject() {
        return *(static_cast<T *>(m_exec_obj));
    }

 protected:

    void started();
    void progress();
    void completed(bool success);
    virtual void dropImp() = 0;
    virtual const std::string getNameImp() const = 0;

    void setPrivateActivatePreExecObj(Object obj);

 protected:
    virtual bool executeImp() = 0;
    virtual void startedImp() = 0;
    virtual void progressImp() = 0;
    virtual void completedImp(bool success) = 0;
    virtual void cancelImp() = 0;
    virtual bool groupCompletedCount() = 0;

    virtual void activatePreExecOpImp();
    virtual bool containingSubCommands();

 private:
    CommandRunner *m_cmd_runner;
    std::string m_name_suffix;
    bool m_done;
    bool m_success;
    bool m_no_way_back;
    bool m_confirmed;
    bool m_canceled;
    bool m_continue_after_failure;
    bool (*m_pre_exec_op)(MissionCommand &);
    Object m_pre_exec_obj;
    Object m_pre_exec_priv_obj;
    bool (*m_post_exec_op)(MissionCommand &, bool);
    Object m_post_exec_obj;
    Object m_exec_obj;
    void (*m_pre_cancel_op)(MissionCommand &);
    Object m_pre_cancel_obj;
    bool m_print_in_progress;
};

class MissionBarrier : public MissionCommand {
 public:
    MissionBarrier();
    virtual ~MissionBarrier();
    void notifyEnded(bool success, MissionCommand &command);
    void addBarrieredMission();
    virtual void dropImp() override;
    virtual bool executeImp() override;
    virtual void startedImp() override;
    virtual void progressImp() override;
    virtual void completedImp(bool success) override;
    virtual void cancelImp();
    virtual bool groupCompletedCount();
    virtual const std::string getNameImp() const override;

 private:
    uint32_t m_ended;
    uint32_t m_running;
    bool m_started_exe;
    bool m_group_success;
};

class BlockableCommand : public MissionCommand {
 public:
    BlockableCommand();
    virtual ~BlockableCommand();
    bool init(CommandRunner &listener, std::string name_suffix);
    void setBarrier(MissionBarrier &barrier);

 protected:

    virtual void dropImp();
    virtual bool groupCompletedCount();
    bool shouldContinue();

 private:
    MissionBarrier *m_barrier;
};

class BlockableCommandCommandRunner : public BlockableCommand, public CommandRunner {
 public:
    BlockableCommandCommandRunner() = default;
    ~BlockableCommandCommandRunner() override = default;
 protected:
    bool initBlockableCommandCommandRunner(CommandRunner &listener, std::string name_suffix);
    //cmd
    void startedImp() override;
    void progressImp() override;
    void completedImp(bool success) override;
    void cancelImp() override;
    //cmd runner
    void commandFailedImp(commands::MissionCommand &command) override;
    void executeEndedImp(bool success) override;
    void progressUpdatedImp() override;
};

class LoopCommand : public BlockableCommand, public infra::TimerHandler {
 public:
    LoopCommand();
    virtual ~LoopCommand();

    bool init(CommandRunner &listener, std::string name_suffix);
    void setLoopBody(uint32_t count, Object object, void(*loop_boody)(MissionCommand &));
    void setBreakCondition(bool(*predicate)(MissionCommand &));
    void addLoopCommand(MissionCommand &command);
    const uint32_t geCmdLeftCount() const { return m_itr_count; }
 protected:
    virtual void activatePreExecOpImp() override;
    virtual bool containingSubCommands() override;
    virtual void dropImp() override;
    virtual bool executeImp() override;
    virtual void startedImp() override;
    virtual void progressImp() override;
    virtual void cancelImp() override;
    virtual void completedImp(bool success) override;
    virtual const std::string getNameImp() const override;

 private:
    uint32_t m_itr_count;
    bool m_have_more_iters;
    bool (*m_predicate)(MissionCommand &);
    void (*m_loop_buddy)(MissionCommand &);
    MissionCommand *m_last_command_added;
    infra::Timer m_timer;
 public:
    void expired();
    static const double ITERATION_WAIT_DURATION;
};

class WaitCommand : public BlockableCommand, public infra::TimerHandler {
 public:
    WaitCommand();
    virtual ~WaitCommand();

    bool init(CommandRunner &listener, std::string name_suffix);
    void setDuration(double duration, uint32_t retries = 0);
    void stopTimer();

 protected:

    virtual void dropImp() override;
    virtual bool executeImp() override;
    virtual void startedImp() override;
    virtual void progressImp() override;
    virtual void cancelImp() override;
    virtual void completedImp(bool success) override;
    virtual const std::string getNameImp() const override;
    virtual void executeOperation(uint32_t count);
    virtual bool getOperationResult();

 private:
    double m_duration;
    uint32_t m_retries;
    uint32_t m_op_count;
    infra::Timer m_timer;
    static const double DEFAULT_WAIT_DURATION;
 public:
    void expired();
};

class FailCommand : public WaitCommand {
 public:
    FailCommand();
    virtual ~FailCommand();
    void setError(const std::string &errorCode);

 protected:
    virtual const std::string getNameImp() const override;
    virtual void completedImp(bool success) override;
    virtual bool getOperationResult() override;

 private:
    std::string m_error_code;
};

class NoopCommand : public WaitCommand {
 public:
    NoopCommand();
    virtual ~NoopCommand();

 protected:

    virtual const std::string getNameImp() const override;
};

class MarkerCommand : public WaitCommand {
 public:
    MarkerCommand();
    virtual ~MarkerCommand();

 protected:

    virtual const std::string getNameImp() const override;
};

class ImageSaverCommand : public MissionCommand {
 public:
    ImageSaverCommand();
    virtual ~ImageSaverCommand();
    void setCameraEnumString(int cameraEnum);

 protected:
    virtual void dropImp() override;
    virtual const std::string getNameImp() const override;
    virtual bool executeImp() override;
    virtual void startedImp() override;
    virtual void progressImp() override;
    virtual void completedImp(bool success) override;
    virtual void cancelImp() override;
    virtual bool groupCompletedCount() override;

 private:
    int m_camera_enum;
};

}
#endif // __COMMANDER_H__
