/* 
 * Copyright (C)2014  iCub Facility - Istituto Italiano di Tecnologia
 * Author: Marco Randazzo
 * email:  marco.randazzo@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <stdio.h>
#include <fstream>
#include <iterator>
#include <yarp/os/RpcClient.h>
#include <yarp/os/YarprunLogger.h>

using namespace yarp::os;
using namespace yarp::os::YarprunLogger;
using namespace std;

const std::string RED    ="\033[01;31m";
const std::string GREEN  ="\033[01;32m";
const std::string YELLOW ="\033[01;33m";
const std::string BLUE   ="\033[01;34m";
const std::string CLEAR  ="\033[00m";

const std::string RED_ERROR      = RED+"ERROR"+CLEAR;
const std::string YELLOW_WARNING = YELLOW+"WARNING"+CLEAR;

void LogEntry::clear_logEntries()
{
    entry_list.clear();
    logInfo.clear();
    last_read_message=-1;
}

void LogEntry::setLogEntryMaxSize(int size)
{
    entry_list_max_size = size;
    entry_list.reserve(entry_list_max_size);
    clear_logEntries();
}

void LogEntry::setLogEntryMaxSizeEnabled (bool enable)
{  
    entry_list_max_size_enabled=enable;
}

bool LogEntry::append_logEntry(MessageEntry entry)
{
    if (logInfo.logsize >= entry_list_max_size && entry_list_max_size_enabled)
    {
        //printf("WARNING: exceeded entry_list_max_size=%d\n",entry_list_max_size);
        return false;
    }
    entry_list.push_back(entry);
    logInfo.logsize++;
    return true;
}

void LogEntryInfo::clear()
{
    logsize=0;
    number_of_debugs=0;
    number_of_errors=0;
    number_of_infos=0;
    number_of_warnings=0;
    highest_error=LOGLEVEL_UNDEFINED;
}

LogLevelEnum  LogEntryInfo::getLastError()
{
    return highest_error;
}

void LogEntryInfo::clearLastError()
{
    highest_error=LOGLEVEL_UNDEFINED;
}

void LogEntryInfo::setNewError(LogLevelEnum level)
{
    if      (level==LOGLEVEL_ERROR)   number_of_errors++;
    else if (level==LOGLEVEL_WARNING) number_of_warnings++;
    else if (level==LOGLEVEL_DEBUG)   number_of_debugs++;
    else if (level==LOGLEVEL_INFO)    number_of_infos++;
    if (level>highest_error) highest_error=level;
}

void LoggerEngine::discover  (std::list<std::string>& ports)
{
    RpcClient p;
    string logger_portname = log_updater->getPortName();
    p.open(logger_portname+"/discover");
    std::string yarpservername = yarp::os::Network::getNameServerName();
    yarp::os::Network::connect(logger_portname+"/discover",yarpservername.c_str());
    Bottle cmd,response;
    cmd.addString("bot");
    cmd.addString("list");
    p.write(cmd,response);
    printf ("%s\n\n", response.toString().c_str());
    int size = response.size();
    for (int i=1; i<size; i++) //beware: skip i=0 is intentional!
    {
        Bottle* n1 = response.get(i).asList();
        if (n1 && n1->get(0).toString()=="port")
        {
            Bottle* n2 = n1->get(1).asList();
            if (n2 && n2->get(0).toString()=="name")
            {
                char* off = 0;
                off = std::strstr((char*)(n2->get(1).toString().c_str()), "/log/");
                if (off > 0)
                {
                    std::string logport = n2->get(1).toString();
                    printf ("%s\n", logport.c_str());
                    ports.push_back(logport);
                }
            }
        }
    }

    std::list<std::string>::iterator ports_it;
    for (ports_it=ports.begin(); ports_it!=ports.end(); ports_it++)
    {
        LogEntry entry;
        entry.logInfo.port_complete = (*ports_it);
        yarp::os::Contact contact = yarp::os::Network::queryName(entry.logInfo.port_complete);
        if (contact.isValid())
        {
            entry.logInfo.ip_address = contact.getHost();
        }
        else
        {
            printf("ERROR: invalid contact: %s", entry.logInfo.port_complete.c_str());
        }
        std::istringstream iss(*ports_it);
        std::string token;
        getline(iss, token, '/');
        getline(iss, token, '/');
        getline(iss, token, '/'); entry.logInfo.port_prefix  = "/"+ token;
        getline(iss, token, '/'); entry.logInfo.process_name = token;
        getline(iss, token, '/'); entry.logInfo.process_pid  = token;
       
        std::list<LogEntry>::iterator it;
        this->log_updater->mutex.wait();
        bool found = false;
        for (it = this->log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
        {
            if (it->logInfo.port_complete==entry.logInfo.port_complete)
            {
                found=true; break;
            }
        }
        if (found==false)
        {
            log_updater->log_list.push_back(entry);
        }
        this->log_updater->mutex.post();
    }
}

void LoggerEngine::connect (const std::list<std::string>& ports)
{
    yarp::os::ContactStyle style;
    style.timeout=1.0;
    style.quiet=true;

    std::list<std::string>::const_iterator it;
    for (it = ports.begin(); it != ports.end(); it++)
    {
        if (yarp::os::Network::exists(it->c_str(),style) == true)
        {
            yarp::os::Network::connect(it->c_str(),this->log_updater->getPortName().c_str());
        }
    }
}

std::string LoggerEngine::logger_thread::getPortName()
{
    return logger_portName;
}

LoggerEngine::logger_thread::logger_thread (std::string _portname, int _rate,  int _log_list_max_size) : RateThread(_rate)
{
        logger_portName              = _portname;
        log_list_max_size            = _log_list_max_size;
        listen_to_LOGLEVEL_INFO      = true;
        listen_to_LOGLEVEL_DEBUG     = true;
        listen_to_LOGLEVEL_ERROR     = true;
        listen_to_LOGLEVEL_WARNING   = true;
        listen_to_LOGLEVEL_UNDEFINED = true;
        unknown_format_received      = 0;
}

void LoggerEngine::logger_thread::run()
{
    //if (is_discovering()==true)
    {
        //yarp::os::Network::
    }

    //yarp::os::Time::delay(0.001);
    std::time_t machine_current_time = std::time(NULL);
    if (logger_port.getInputCount()>0)
    {
        int bufferport_size = logger_port.getPendingReads();

        while (bufferport_size>0)
        {
            Bottle *b = logger_port.read(); //this is blocking
            bufferport_size = logger_port.getPendingReads();

            if (b==NULL)
            {
                fprintf (stderr, "ERROR: something strange happened here, bufferport_size = %d!\n",bufferport_size);
                return;
            }

            if (b->size()!=2) 
            {
                fprintf (stderr, "ERROR: unknown log format!\n");
                unknown_format_received++;
                continue;
            }

            std::string bottlestring = b->toString();
            std::string header = b->get(0).asString();
            MessageEntry body;
            std::string s = b->get(1).asString();

            body.text = s;
            char ttstr [20];
            static int count=0;
            sprintf(ttstr,"%d",count++);
            body.timestamp = string(ttstr);
            body.level = LOGLEVEL_UNDEFINED;

            int str = s.find('[',0);
            int end = s.find(']',0);
            if (str==std::string::npos || end==std::string::npos )
            {
                body.level = LOGLEVEL_UNDEFINED;
            }
            else if (str==0)
            {
                std::string level = s.substr(str,end+1);
                body.level = LOGLEVEL_UNDEFINED;
                if      (level == "[INFO]")      body.level = LOGLEVEL_INFO;
                else if (level == "[DEBUG]")     body.level = LOGLEVEL_DEBUG;
                else if (level == "[WARNING]")   body.level = LOGLEVEL_WARNING;
                else if (level == "[ERROR]")     body.level = LOGLEVEL_ERROR;
                body.text = s.substr(end+1);
            }
            else 
            {
                if (s.at(0) == (const char)'\033')
                {
                    //header example "\033[01;31m"
                    if      (s.at(8) == (const char)'W') body.level = LOGLEVEL_WARNING;
                    else if (s.at(8) == (const char)'E') body.level = LOGLEVEL_ERROR;
                }
                else
                {
                    //int ss = strncmp(s.c_str,RED.c_str(),RED.size());
                    body.level = LOGLEVEL_UNDEFINED;
                }
            }

            if (body.level == LOGLEVEL_INFO      && listen_to_LOGLEVEL_INFO      == false) {continue;}
            if (body.level == LOGLEVEL_DEBUG     && listen_to_LOGLEVEL_DEBUG     == false) {continue;}
            if (body.level == LOGLEVEL_WARNING   && listen_to_LOGLEVEL_WARNING   == false) {continue;}
            if (body.level == LOGLEVEL_ERROR     && listen_to_LOGLEVEL_ERROR     == false) {continue;}
            if (body.level == LOGLEVEL_UNDEFINED && listen_to_LOGLEVEL_UNDEFINED == false) {continue;}

            this->mutex.wait();
            LogEntry entry;
            entry.logInfo.port_complete = header;
            entry.logInfo.port_complete.erase(0,1);
            entry.logInfo.port_complete.erase(entry.logInfo.port_complete.size()-1);
            std::istringstream iss(header);
            std::string token;
            getline(iss, token, '/');
            getline(iss, token, '/');
            getline(iss, token, '/'); entry.logInfo.port_prefix  = "/"+ token;
            getline(iss, token, '/'); entry.logInfo.process_name = token;
            getline(iss, token, '/'); entry.logInfo.process_pid  = token.erase(token.size()-1);
        
            std::list<LogEntry>::iterator it;
            for (it = log_list.begin(); it != log_list.end(); it++)
            {
                if (it->logInfo.port_complete==entry.logInfo.port_complete)
                {
                    it->logInfo.setNewError(body.level);
                    it->logInfo.last_update=machine_current_time;
                    it->append_logEntry(body);
                    break;
                }
            }
            if (it == log_list.end())
            {
                if (log_list.size() < log_list_max_size || log_list_max_size_enabled==false )
                {
                    yarp::os::Contact contact = yarp::os::Network::queryName(entry.logInfo.port_complete);
                    if (contact.isValid())
                    {
                        entry.logInfo.setNewError(body.level);
                        entry.logInfo.ip_address = contact.getHost();
                    }
                    else
                    {
                        printf("ERROR: invalid contact: %s", entry.logInfo.port_complete.c_str());
                    };
                    entry.append_logEntry(body);
                    entry.logInfo.last_update=machine_current_time;
                    log_list.push_back(entry);
                }
                //else
                //{
                //    printf("WARNING: exceeded log_list_max_size=%d\n",log_list_max_size);
                //}
            }
        
            this->mutex.post();
        }
    }

}

//public methods
bool LoggerEngine::start_logging()
{
    if (logging == true)
    {
        fprintf(stderr,"logger is already running, listening on port %s\n", log_updater->getPortName().c_str());
        return true;
    }

    if (log_updater->logger_port.open(log_updater->getPortName().c_str())==true)
    {
        fprintf(stdout,"Logger successfully started, listening on port %s\n", log_updater->getPortName().c_str());
    }
    else
    {
        fprintf(stderr,"Unable to start logger: port %s is unavailable\n", log_updater->getPortName().c_str());
        return false;
    }
    log_updater->logger_port.resume();
    log_updater->logger_port.setStrict();
    logging=true;
    log_updater->start();
    return true;
}

void LoggerEngine::logger_thread::threadRelease()
{
    logger_port.interrupt();
    logger_port.close();
}

void LoggerEngine::stop_logging()
{
    if (log_updater == NULL) return;
    
    logging=false;
    if (log_updater->isRunning()==true) log_updater->stop();
}

void LoggerEngine::start_discover()
{
    if (log_updater == NULL) return;
    
    log_updater->start();
    discovering=true;
}

void LoggerEngine::stop_discover()
{
    discovering=false;
}

LoggerEngine::LoggerEngine(std::string portName)
{
    int thread_rate = 10;
    log_updater=new logger_thread (portName,thread_rate);
    logging = false;
    discovering = false;
}

LoggerEngine::~LoggerEngine()
{
    this->stop_logging();
    if (log_updater!=0)
    {
        delete log_updater;
        log_updater = 0;
    }
}

int  LoggerEngine::get_num_of_processes()
{
    if (log_updater == NULL) return 0;
    
    return log_updater->logger_port.getInputCount();
}

void LoggerEngine::get_infos (std::list<LogEntryInfo>& infos)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        infos.push_back(it->logInfo);
    }
    log_updater->mutex.post();
}

void LoggerEngine::get_messages (std::list<MessageEntry>& messages)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        messages.insert(messages.end(), it->entry_list.begin(), it->entry_list.end());
    }
    log_updater->mutex.post();
}

void LoggerEngine::get_messages_by_port_prefix    (std::string  port,  std::list<MessageEntry>& messages,  bool from_beginning)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.port_prefix == port)
        {
            //messages = (it->entry_list);
            if (it->last_read_message==-1)
            {
                from_beginning=true;
            }
            if (from_beginning==true) 
            {
                it->last_read_message = 0;
            }
            int i=it->last_read_message;
            for (; i<(int)it->entry_list.size(); i++)
            {
                messages.push_back(it->entry_list[i]);
            }
            it->last_read_message=i;
            break;
        }
    }
    log_updater->mutex.post();
}

void LoggerEngine::clear_messages_by_port_complete    (std::string  port)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.port_complete == port)
           {
               it->clear_logEntries();
               break;
           }
    }
    log_updater->mutex.post();
}

void LoggerEngine::get_messages_by_port_complete    (std::string  port,  std::list<MessageEntry>& messages,  bool from_beginning)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.port_complete == port)
        {
            //messages = (it->entry_list);
            if (it->last_read_message==-1)
            {
                from_beginning=true;
            }
            if (from_beginning==true) 
            {
                it->last_read_message = 0;
            }
            int i=it->last_read_message;
            int size = (int)it->entry_list.size();
            for (; i<size; i++)
            {
                messages.push_back(it->entry_list[i]);
            }
            it->last_read_message=i;
            break;
        }
    }
    log_updater->mutex.post();
}

void LoggerEngine::get_messages_by_process (std::string  process,  std::list<MessageEntry>& messages,  bool from_beginning)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.process_name == process)
        {
            //messages = (it->entry_list);
            if (it->last_read_message==-1)
            {
                from_beginning=true;
            }
            if (from_beginning==true) 
            {
                it->last_read_message = 0;
            }
            int i=it->last_read_message;
            for (; i<(int)it->entry_list.size(); i++)
            {
                messages.push_back(it->entry_list[i]);
            }
            it->last_read_message=i;
            break;
        }
    }
    log_updater->mutex.post();
}

void LoggerEngine::get_messages_by_pid     (std::string pid, std::list<MessageEntry>& messages,  bool from_beginning)
{
    if (log_updater == NULL) return;
    
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.process_pid == pid)
        {
            //messages = (it->entry_list);
            if (it->last_read_message==-1)
            {
                from_beginning=true;
            }
            if (from_beginning==true) 
            {
                it->last_read_message = 0;
            }
            int i=it->last_read_message;
            for (; i<(int)it->entry_list.size(); i++)
            {
                messages.push_back(it->entry_list[i]);
            }
            it->last_read_message=i;
            break;
        }
    }
    log_updater->mutex.post();
}

const std::list<MessageEntry> filter_by_level (int level, const std::list<MessageEntry>& messages)
{
    std::list<MessageEntry> ret;
    std::list<MessageEntry>::const_iterator it;
    for (it = messages.begin(); it != messages.end(); it++)
    {
        if (it->level==level)
            ret.push_back(*it);
    }
    return ret;
}

void LoggerEngine::set_listen_option               (LogLevelEnum logLevel, bool enable)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();
         if (logLevel == LOGLEVEL_ERROR)      {log_updater->listen_to_LOGLEVEL_ERROR=enable;}
    else if (logLevel == LOGLEVEL_WARNING)    {log_updater->listen_to_LOGLEVEL_WARNING=enable;}
    else if (logLevel == LOGLEVEL_DEBUG)      {log_updater->listen_to_LOGLEVEL_DEBUG=enable;}
    else if (logLevel == LOGLEVEL_INFO)       {log_updater->listen_to_LOGLEVEL_INFO=enable;}
    else if (logLevel == LOGLEVEL_UNDEFINED)  {log_updater->listen_to_LOGLEVEL_UNDEFINED=enable;}
    log_updater->mutex.post();
}

bool LoggerEngine::get_listen_option               (LogLevelEnum logLevel)
{
    if (log_updater == NULL) return false;
         if (logLevel == LOGLEVEL_ERROR)     {return log_updater->listen_to_LOGLEVEL_ERROR;}
    else if (logLevel == LOGLEVEL_WARNING)   {return log_updater->listen_to_LOGLEVEL_WARNING;}
    else if (logLevel == LOGLEVEL_DEBUG)     {return log_updater->listen_to_LOGLEVEL_DEBUG;}
    else if (logLevel == LOGLEVEL_INFO)      {return log_updater->listen_to_LOGLEVEL_INFO;}
    else if (logLevel == LOGLEVEL_UNDEFINED) {return log_updater->listen_to_LOGLEVEL_UNDEFINED;}
    else return false;
}

void LoggerEngine::export_log_to_text_file   (std::string  filename, std::string portname)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();
    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        if (it->logInfo.port_complete == portname)
        {
            ofstream file1;
            file1.open(filename.c_str());
            std::vector<MessageEntry>::iterator it1;
            for (it1 = it->entry_list.begin(); it1 != it->entry_list.end(); it1++)
            {
                file1 << it1->timestamp << " " << it1->level << " " << it1->text << " " << std::endl;
            }
            file1.close();
        }
    }
    log_updater->mutex.post();
}

void LoggerEngine::save_all_logs_to_file   (std::string  filename)
{
    if (log_updater == NULL) return;
    
    const int      LOGFILE_VERSION = 1;

    ofstream file1;
    file1.open(filename.c_str());
    bool wasRunning = log_updater->isRunning();
    if (wasRunning) log_updater->stop();
    std::list<LogEntry>::iterator it;
    file1 << LOGFILE_VERSION << std::endl;
    file1 << log_updater->log_list.size() << std::endl;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        file1 << it->logInfo.port_complete << std::endl;
        file1 << it->logInfo.port_prefix << std::endl;
        file1 << it->logInfo.process_name << std::endl;
        file1 << it->logInfo.process_pid << std::endl;
        file1 << it->entry_list.size() << std::endl;
        std::vector<MessageEntry>::iterator it1;
        for (it1 = it->entry_list.begin(); it1 != it->entry_list.end(); it1++)
        {
            file1 << it1->timestamp << std::endl;
            file1 << it1->level << std::endl;
            file1 << it1->text << std::endl;
        }
    }
    file1.close();
    if (wasRunning) log_updater->start();
}

void LoggerEngine::load_all_logs_from_file   (std::string  filename)
{
    if (log_updater == NULL) return;
    
    const int      LOGFILE_VERSION = 1;

    ifstream file1;
    file1.open(filename.c_str());
    int log_file_version;
    bool wasRunning = log_updater->isRunning();
    if (wasRunning) log_updater->stop();
    file1 >> log_file_version;
    if (log_file_version == LOGFILE_VERSION)
    {
        int size_log_list;
        file1 >> size_log_list;
        log_updater->log_list.clear();
        for (int i=0; i< size_log_list; i++)
        {
            LogEntry l_tmp;
            file1 >> l_tmp.logInfo.port_complete;
            file1 >> l_tmp.logInfo.port_prefix;
            file1 >> l_tmp.logInfo.process_name;
            file1 >> l_tmp.logInfo.process_pid;
            int size_entry_list;
            file1 >> size_entry_list;
            for (int j=0; j< size_entry_list; j++)
            {
                MessageEntry m_tmp;
                file1 >> m_tmp.timestamp;
                int tmp_level; 
                file1 >> tmp_level;
                m_tmp.level = static_cast<LogLevelEnum>(tmp_level);
                file1 >> m_tmp.text;
                l_tmp.entry_list.push_back(m_tmp);
            }
            log_updater->log_list.push_back(l_tmp);
        }
    }
    file1.close();
    if (wasRunning) log_updater->start();
}

void LoggerEngine::set_log_lines_max_size (bool  enabled,  int new_size)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();

    std::list<LogEntry>::iterator it;
    for (it = log_updater->log_list.begin(); it != log_updater->log_list.end(); it++)
    {
        it->setLogEntryMaxSize(new_size);
        it->setLogEntryMaxSizeEnabled(enabled);
    }

    log_updater->mutex.post();
}


void LoggerEngine::set_log_list_max_size           (bool  enabled,  int new_size)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();
    log_updater->log_list_max_size_enabled = enabled;
    log_updater->log_list_max_size = new_size;
    log_updater->mutex.post();
}

void LoggerEngine::get_log_lines_max_size          (bool& enabled, int& current_size)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();
    if (log_updater->log_list.empty() == true) return;

    std::list<LogEntry>::iterator it = log_updater->log_list.begin();
    current_size = it->getLogEntryMaxSize();
    enabled = it->getLogEntryMaxSizeEnabled();
    log_updater->mutex.post();
}

void LoggerEngine::get_log_list_max_size           (bool& enabled, int& current_size)
{
    if (log_updater == NULL) return;
    log_updater->mutex.wait();
    enabled=log_updater->log_list_max_size_enabled;
    current_size=log_updater->log_list_max_size;
    log_updater->mutex.post();
}