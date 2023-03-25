#include "commands/modbus_commands.h"

namespace commands {

bool ModbusCommand::init(CommandRunner &listener, modbus &modbus_instance, std::string name_suffix) {
    if (BlockableCommand::init(listener, name_suffix)) {
        m_modbus_handler = &modbus_instance;
        return true;
    }
    return false;
};

// =========================================================================================== ReadCoils

void ReadCoils::setArgs(int address, int amount, bool buffer) {
        m_address = address;
        m_amount = amount;
        m_buffer = buffer;
        m_args_set = true;
    };


bool ReadCoils::executeImp() {
    if(m_args_set) {
        int status = m_modbus_handler->modbus_read_coils(m_address, m_amount, &m_buffer);
        m_args_set = false;
        bool success = ((status==-1) ? true : false); //status >> 0 - ok, -1 - not ok
        return success; 
    } else return NOT_SET;
};

// =========================================================================================== ReadInputBits

void ReadInputBits::setArgs(int address, int amount, bool buffer) {
    m_address = address;
    m_amount = amount;
    m_buffer = buffer;
    m_args_set = true;
};

bool ReadInputBits::executeImp() {
    if(m_args_set) {
        int status = m_modbus_handler->modbus_read_input_bits(m_address, m_amount, &m_buffer);
        m_args_set = false;
        bool success = ((status==0) ? true : false); //status >> 0 - ok, -1 - not ok
        return success; 
    } else return NOT_SET;
};

// =========================================================================================== ReadSingleHoldingRegister

ReadSingleHoldingRegister::~ReadSingleHoldingRegister() { 
};

void ReadSingleHoldingRegister::setArgs(int address) {
    m_address = address;
    m_amount = 1;
    m_args_set = true;
};

bool ReadSingleHoldingRegister::executeImp() {
    if(m_args_set) {
        int status = m_modbus_handler->modbus_read_holding_registers(m_address, m_amount, m_buffer_ptr);
        m_args_set = false;
        bool success = ((status==0) ? true : false); //status >> 0 - ok, -1 - not ok
        return success; 
    } else return NOT_SET;
};

uint16_t ReadSingleHoldingRegister::getResult() { 
    return m_buffer_ptr[0];
}

// =========================================================================================== ReadHoldingRegisters

// ReadHoldingRegisters::~ReadHoldingRegisters() { 
//     delete[] m_buffer_ptr;
//     if(!m_result_ptr)
//         delete[] m_result_ptr;
// };

// void ReadHoldingRegisters::setArgs(int address, int amount) {
//     m_address = address;
//     m_amount = amount;
//     m_buffer_ptr = new uint16_t[amount];
//     m_args_set = true;
// };

// bool ReadHoldingRegisters::executeImp() {
//     if(m_args_set) {
//         int status = m_modbus_handler->modbus_read_holding_registers(m_address, m_amount, m_buffer_ptr);
//         m_args_set = false;
//         return status; // 0 - ok, -1 - not ok 
//     } else return NOT_SET;
// };

// uint16_t* ReadHoldingRegisters::getResult() { 
//     if(!m_result_ptr)
//         delete[] m_result_ptr;
//     m_result_ptr = new uint16_t[m_amount];
//     *m_result_ptr = *m_buffer_ptr;
//     delete[] m_buffer_ptr;
//     m_buffer_ptr = nullptr;
//     return m_result_ptr; 
// }

// =========================================================================================== ReadInputRegisters

// ReadInputRegisters::~ReadInputRegisters() { 
//     delete[] m_buffer_ptr;
//     if(!m_result_ptr)
//         delete[] m_result_ptr;
// };

// void ReadInputRegisters::setArgs(int address, int amount, uint16_t buffer) {
//     m_address = address;
//     m_amount = amount;
//     m_buffer_ptr = new uint16_t[amount];
//     m_args_set = true;
// };

// bool ReadInputRegisters::executeImp() {
//     if(m_args_set) {
//         int status = m_modbus_handler->modbus_read_input_registers(m_address, m_amount, m_buffer_ptr);
//         m_args_set = false;
//         return status; // 0 - ok, -1 - not ok 
//     } else return NOT_SET;
// };

// uint16_t* ReadInputRegisters::getResult() { 
//     if(!m_result_ptr)
//         delete[] m_result_ptr;
//     m_result_ptr = new uint16_t[m_amount];
//     *m_result_ptr = *m_buffer_ptr;
//     delete[] m_buffer_ptr;
//     m_buffer_ptr = nullptr;
//     return m_result_ptr; 
// }

// =========================================================================================== WriteCoil

void WriteCoil::setArgs(int address, bool buffer) {
    m_address = address;
    m_buffer = buffer;
    m_args_set = true;
};

bool WriteCoil::executeImp() {
    if(m_args_set) {
        int status = m_modbus_handler->modbus_write_coil(m_address, m_buffer);
        m_args_set = false;
        bool success = ((status==0) ? true : false); //status >> 0 - ok, -1 - not ok
        return success;  
    } else return NOT_SET;
};

// =========================================================================================== WriteSingleRegister

void WriteSingleRegister::setArgs(int address, uint16_t buffer) {
        m_address = address;
        m_buffer[0] = buffer;
        m_args_set = true;
    };

bool WriteSingleRegister::executeImp() {
    if(m_args_set) {
        int status = m_modbus_handler->modbus_write_registers(m_address, 5, m_buffer);
        // m_buffer_ptr = new uint16_t[amount];
        m_args_set = false;
        bool success = ((status==0) ? true : false); //status >> 0 - ok, -1 - not ok
        return success;  
    } else return NOT_SET;
};

// =========================================================================================== WriteMultipleCoils

// void WriteMultipleCoils::setArgs(int address, int amount, bool *buffer) {
//     m_address = address;
//     m_amount = amount;
//     m_buffer_ptr = new bool[amount];
//     for(int i=0 ; i < amount ; i++) 
//         m_buffer_ptr[i] = *(buffer + i);
//     m_args_set = true;
// };

// bool WriteMultipleCoils::executeImp() {
//     if(m_args_set) {
//         int status = m_modbus_handler->modbus_write_coils(m_address, m_amount, m_buffer_ptr);
//         m_args_set = false;
//         return status; // 0 - ok, -1 - not ok 
//     } else return NOT_SET;
// };

// =========================================================================================== WriteMultipleRegisters

// void WriteMultipleRegisters::setArgs(int address, int amount, uint16_t *buffer) {
//     m_address = address;
//     m_amount = amount;
//     m_buffer_ptr = new uint16_t[amount];
//     for(int i=0 ; i < amount ; i++) 
//         m_buffer_ptr[i] = *(buffer + i);
//     m_args_set = true;
// };

// bool WriteMultipleRegisters::executeImp() {
//     if(m_args_set) {
//         int status = m_modbus_handler->modbus_write_registers(m_address, m_amount, m_buffer_ptr);
//         m_args_set = false;
//         return status; // 0 - ok, -1 - not ok 
//     } else return NOT_SET;
// };

};