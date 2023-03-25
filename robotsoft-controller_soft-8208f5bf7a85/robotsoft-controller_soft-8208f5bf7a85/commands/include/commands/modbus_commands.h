#ifndef __MODBUS_COMMANDS_H__
#define __MODBUS_COMMANDS_H__
#include "commands/commander.h"
#include "modbus/modbus.h"
#define NOT_SET 101;

namespace commands {

class ModbusCommand : public BlockableCommand {
 public:
   ModbusCommand() :  m_args_set(false), BlockableCommand() {};
   virtual ~ModbusCommand() = default;
   bool init(CommandRunner &listener, modbus &modbus_instance, std::string name_suffix);
   virtual void startedImp() override {};
   virtual void progressImp() override {};
   virtual void completedImp(bool success) override {};
   virtual void cancelImp() override {};
   virtual bool groupCompletedCount() override {};
   void completeCmd(bool success=true) { completed(success); }

 protected:
   int m_address;
   bool m_args_set;
   std::string m_ip;
   int m_port;
   modbus *m_modbus_handler;
};

class ReadCoils : public ModbusCommand {
 public:
   ReadCoils() : ModbusCommand() {};
   virtual ~ReadCoils();
   virtual void setArgs(int address, int amount, bool buffer);
   virtual bool executeImp() override;
   virtual bool getResult() { return m_buffer; }
   virtual const std::string getNameImp() const override {return "ReadCoilsCommand";}

 private:
   int m_amount;
   bool m_buffer;
};

class ReadInputBits : public ModbusCommand {
 public:
   ReadInputBits() : ModbusCommand() {};
   virtual ~ReadInputBits();
   virtual void setArgs(int address, int amount, bool buffer);
   virtual bool executeImp() override;
   virtual bool getResult() { return m_buffer; }
   virtual const std::string getNameImp() const override {return "ReadInputBitsCommand";}

 private:
   int m_amount;
   bool m_buffer;
};

class ReadSingleHoldingRegister : public ModbusCommand {
 public:
   ReadSingleHoldingRegister() : ModbusCommand() { };
   virtual ~ReadSingleHoldingRegister();
   virtual void setArgs(int address);
   virtual bool executeImp() override;
   virtual uint16_t getResult();
   virtual const std::string getNameImp() const override {return "ReadHoldingRegistersCommand";}

 private:
   int m_amount;
   uint16_t m_buffer_ptr[1];
};

//=================================//
//    DO NOT USE UNLESS YOU MUST   //
//=================================//
// class ReadHoldingRegisters : public ModbusCommand {
//  public:
//    ReadHoldingRegisters() : m_buffer_ptr(nullptr), ModbusCommand() { };
//    virtual ~ReadHoldingRegisters();
//    virtual void setArgs(int address, int amount);
//    virtual bool executeImp() override;
//    virtual uint16_t* getResult();
//    virtual const std::string getNameImp() const override {return "ReadHoldingRegistersCommand";}

//  private:
//    int m_amount;
//    uint16_t *m_buffer_ptr;
//    uint16_t *m_result_ptr;
// };

//=================================//
//    DO NOT USE UNLESS YOU MUST   //
//=================================//
// class ReadInputRegisters : public ModbusCommand {
//  public:
//    ReadInputRegisters() : m_buffer_ptr(nullptr), ModbusCommand() {};
//    virtual ~ReadInputRegisters();
//    virtual void setArgs(int address, int amount, uint16_t buffer);
//    virtual bool executeImp() override;
//    virtual uint16_t* getResult();
//    virtual const std::string getNameImp() const override {return "ReadInputRegistersCommand";}

//  private:
//    int m_amount;
//    uint16_t *m_buffer_ptr;
//    uint16_t *m_result_ptr;
// };

class WriteCoil : public ModbusCommand {
 public:
   WriteCoil() : ModbusCommand() {};
   virtual ~WriteCoil();
   virtual void setArgs(int address, bool buffer);
   virtual bool executeImp() override;
   virtual const std::string getNameImp() const override {return "WriteCoilCommand";}

 private:
   bool m_buffer;
};

class WriteSingleRegister : public ModbusCommand {
 public:
   WriteSingleRegister() : ModbusCommand() {};
   virtual ~WriteSingleRegister() {};
   virtual void setArgs(int address, uint16_t buffer);
   virtual bool executeImp() override;
   virtual const std::string getNameImp() const override {return "WriteSingleRegisterCommand";}

 private:
   uint16_t m_buffer[5] = {0,0,0,0,0};
};

//=================================//
//    DO NOT USE UNLESS YOU MUST   //
//=================================//
// class WriteMultipleCoils : public ModbusCommand {
//  public:
//     WriteMultipleCoils() : m_buffer_ptr(nullptr), ModbusCommand() {};
//     virtual ~WriteMultipleCoils() { delete[] m_buffer_ptr; };
//     virtual void setArgs(int address, int amount, bool *buffer);
//     virtual bool executeImp() override;
//     virtual const std::string getNameImp() const override {return "WriteMultipleCoilsCommand";}

//  private:
//     int m_address;
//     int m_amount;
//     bool *m_buffer_ptr;
// };

//=================================//
//    DO NOT USE UNLESS YOU MUST   //
//=================================//
// class WriteMultipleRegisters : public ModbusCommand {
//  public:
//     WriteMultipleRegisters() : ModbusCommand() {};
//     virtual ~WriteMultipleRegisters() { delete[] m_buffer_ptr; };
//     virtual void setArgs(int address, int amount, uint16_t *buffer);
//     virtual bool executeImp() override;
//     virtual const std::string getNameImp() const override {return "WriteMultipleRegistersCommand";}

//  private:
//     int m_address;
//     int m_amount;
//     uint16_t *m_buffer_ptr;
// };

}


#endif  //__MODBUS_COMMANDS_H__