#ifndef __PIPE_QUEUE_H__
#define __PIPE_QUEUE_H__

namespace infra {

template<class T>
class ItemSerializer {
 public:
    ItemSerializer() : m_send_fd(-1), m_recv_fd(-1), m_max_ser_size(0), m_ser_deser_buf(nullptr) {}
    virtual ~ItemSerializer() {
        if (!destroy()) {
            Logger::log().error("Failed to destroy serializer");
        }
    };
    bool init(int send_fd, int recv_fd, uint64_t max_ser_size) {
        if (!m_ser_deser_buf) {
            m_ser_deser_buf = new uint8_t[max_ser_size + sizeof(m_max_ser_size)];
        }
        if (!m_ser_deser_buf) {
            Logger::log().error("Failed to allocate serialization buffer. max_ser_size={}", max_ser_size);
            return false;
        }
        m_send_fd = send_fd;
        m_recv_fd = recv_fd;
        m_max_ser_size = max_ser_size;
        return true;
    }
    bool destroy() {
        if (m_ser_deser_buf) {
            delete[] m_ser_deser_buf;
            m_ser_deser_buf = nullptr;
        }
        return true;
    }
    bool sendser(T &item) {
        uint64_t ser_len;
        if ((ser_len = item.serialize(&(m_ser_deser_buf[sizeof(ser_len)]), m_max_ser_size))) {
            memcpy(m_ser_deser_buf, &ser_len, sizeof(ser_len));
            long int ret = write(m_send_fd, m_ser_deser_buf, ser_len + sizeof(ser_len));
            if (ret != (ser_len + sizeof(ser_len))) {
                Logger::log().error("Failed to send item fd={}, ret={}, ser_len={}, errno={}", m_send_fd, ret, ser_len, errno);
                return false;
            }
            return true;
        }
        Logger::log().error("Failed to serialize, m_max_read_size={}", m_max_ser_size);
        return false;
    }

    bool recvser(T &item, bool &timedout, uint64_t timeout_milli) {
        timedout = false;
        struct pollfd pollfds;
        pollfds.fd = m_recv_fd;
        pollfds.events = POLLIN;
        int ret = poll(&pollfds, sizeof(pollfds) / sizeof(struct pollfd), timeout_milli);
        if (ret < 0) {
            Logger::log().error("Failed to poll on receive. timeout_milli={}, errno={}", timeout_milli, errno);
#warning "How we handle that?"
            return false;
        } else if (ret == 0) {
            timedout = true;
            return false;
        } else {
            uint64_t info_size = 0;
            if (read_buffer(m_recv_fd, info_size) && (item.deserialize(m_ser_deser_buf, info_size))) {
                return true;
            }
            Logger::log().error("Failed to read buffer or serialize", m_recv_fd);
            return false;
        }
    }
 private:
    bool read_buffer(int fd, uint64_t &info_size) {
        uint64_t read_size;
        long int ret = read(fd, &read_size, sizeof(read_size));
        if (ret < sizeof(read_size)) {
#warning "Make sure we have no terminal cases here."
            Logger::log().error("Failed to read size, fd={}, read_size={}, ret={}, errno={}", fd, read_size, ret, errno);
            return false;
        }
        if (read_size > m_max_ser_size) {
            Logger::log().error("Info length too long. read_size={}, m_max_read_size={}", read_size, m_max_ser_size);
            return false;
        }
#warning "Make sure we have no terminal cases here."
        ret = read(fd, m_ser_deser_buf, read_size);
        if (ret < read_size) {
            Logger::log().error("Failed to read info, fd={}, read_size={}, ret={}, errno={}", fd, read_size, ret, errno);
            return false;
        }
        info_size = read_size;
        return true;
    }
 private:
    int m_send_fd;
    int m_recv_fd;
    uint64_t m_max_ser_size;
    uint8_t *m_ser_deser_buf;
};

template<class T>
class PipeQueue {
 public:
    PipeQueue() {
        m_parent_pipe_fds[0] = m_parent_pipe_fds[1] = m_child_pipe_fds[0] = m_child_pipe_fds[1] = -1;
    }
    virtual ~PipeQueue() {
        if (!destroy()) {
            Logger::log().error("Failed to destroy PipeQueue");
        }
    }

    bool init(uint64_t max_item_size = ItemSerializer<T>::MAX_SERIALIZE_BUF_LEN) {
        Logger::log().info("PipeQueue::init - started");

        int ret = pipe2(m_parent_pipe_fds, O_NONBLOCK);
        if (ret) {
            Logger::log().error("Failed to open parent pipe errno={}", errno);
            return false;
        }

        Logger::log().info("Opened pipe. m_parent_pipe_fds[0]={}, m_parent_pipe_fds[1]={}", m_parent_pipe_fds[0], m_parent_pipe_fds[1]);

        ret = pipe2(m_child_pipe_fds, O_NONBLOCK);
        if (ret) {
            Logger::log().error("Failed to open child pipe errno={}", errno);
            destroy();
            return false;
        }

        Logger::log().info("Opened pipe. m_child_pipe_fds[0]={}, m_child_pipe_fds[1]={}", m_child_pipe_fds[0], m_child_pipe_fds[1]);

        ret = m_parent_ser.init(m_parent_pipe_fds[1], m_child_pipe_fds[0], max_item_size);
        if (!ret) {
            Logger::log().error("Failed to initialize parent serializer");
            destroy();
            return false;
        }
        m_child_ser.init(m_child_pipe_fds[1], m_parent_pipe_fds[0], max_item_size);
        if (!ret) {
            Logger::log().error("Failed to initialize child serializer");
            destroy();
            return false;
        }

        Logger::log().info("PipeQueue::init - ended");

        return true;
    }

    bool destroy() {
        Logger::log().info("PipeQueue::destroy - started");

        int ret = (m_parent_pipe_fds[0] == -1) ? 0 : close(m_parent_pipe_fds[0]);
        if (ret) {
            Logger::log().error("Failed to close parent pipe 0 errno={}", errno);
        }

        int ret_temp = (m_parent_pipe_fds[1] == -1) ? 0 : close(m_parent_pipe_fds[1]);
        if (ret_temp) {
            ret = ret ? ret : ret_temp;
            Logger::log().error("Failed to close parent pipe 1 errno={}", errno);
        }

        ret_temp = (m_child_pipe_fds[0] == -1) ? 0 : close(m_child_pipe_fds[0]);
        if (ret_temp) {
            ret = ret ? ret : ret_temp;
            Logger::log().error("Failed to close child pipe 0 errno={}", errno);
        }

        ret_temp = (m_child_pipe_fds[1] == -1) ? 0 : close(m_child_pipe_fds[1]);
        if (ret_temp) {
            ret = ret ? ret : ret_temp;
            Logger::log().error("Failed to close child pipe 1 errno={}", errno);
        }

        ret_temp = m_parent_ser.destroy();
        if (ret_temp) {
            ret = ret ? ret : ret_temp;
            Logger::log().error("Failed to destroy parent serializer");
        }

        m_child_ser.destroy();
        if (ret_temp) {
            ret = ret ? ret : ret_temp;
            Logger::log().error("Failed to destroy child serializer");
        }

        Logger::log().info("PipeQueue::destroy - ended");

        return ret;
    }

    bool parentSend(T &item) {
        return m_parent_ser.sendser(item);
    }

    bool parentRecv(T &item, bool &timedout, uint64_t timeout_milli) {
        return m_parent_ser.recvser(item, timedout, timeout_milli);
    }

    bool childSend(T &item) {
        return m_child_ser.sendser(item);
    }

    bool childRecv(T &item, bool &timedout, uint64_t timeout_milli) {
        return m_child_ser.recvser(item, timedout, timeout_milli);
    }

 private:
    int m_parent_pipe_fds[2];
    int m_child_pipe_fds[2];
    ItemSerializer<T> m_parent_ser;
    ItemSerializer<T> m_child_ser;
};
};
#endif // __PIPE_QUEUE_H__
