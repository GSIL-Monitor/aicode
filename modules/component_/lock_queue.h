
#include <boost/thread.hpp>  
#include <list>
template<typename T>
class LockedQueue
{
    public:
        LockedQueue() {}
        ~LockedQueue() 
        {
            if (!m_queue.empty())
            {
                m_queue.clear();
            }

        }

        int PushBack(const T &pt) 
        {
            boost::mutex::scoped_lock lock(m_mutex);
            m_queue.push_back(pt);
            return m_queue.size();
        }

        T PopFront() 
        {
            boost::mutex::scoped_lock lock(m_mutex);
            if (m_queue.size() > 0) 
            {
                T oThread = m_queue.front();
                m_queue.pop_front();
                return oThread;
            }

            return m_nil_obj;
        }

        void Erase(T &Object) 
        {
            boost::mutex::scoped_lock lock(m_mutex);
            typedef typename std::list<T>::iterator iter_thread;
            for (iter_thread it = m_queue.begin(); it != m_queue.end();) 
            {
                if (Object == *it) 
                {
                    m_queue.erase(it++);
                    break;
                }
                else 
                {
                    ++it;
                }
            }
        }

        void Clean()
        {    
            boost::mutex::scoped_lock lock(m_mutex);
            if (!m_queue.empty())
            {
                m_queue.clear();
            }
            return;
        }

        int Length() 
        {
            boost::mutex::scoped_lock lock(m_mutex);
            return m_queue.size();
        }

        bool IsEmpty()
        {
            boost::mutex::scoped_lock lock(m_mutex);
            return m_queue.empty();
        }

    public:
        boost::mutex m_mutex;

    private:
        std::list<T> m_queue;
        T m_nil_obj;
};
