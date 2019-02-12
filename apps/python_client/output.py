# -*- coding: utf-8 -*-
import logging
import os,sys
import queue
import threading
# logging.debug("This is a debug log.")
# logging.info("This is a info log.")
# logging.warning("This is a warning log.")
# logging.error("This is a error log.")
# logging.critical("This is a critical log.")

thread_queque = queue.Queue(1000)

def log_thread_func(args):
    while True:
        log_data = thread_queque.get()
        if log_data is None:
            return

        log_func, log_content = log_data
        log_func(log_content)

log_thread = threading.Thread(target=log_thread_func,args=(1,))

def Log_Init(log_file):
    if os.path.exists(log_file):
        os.unlink(log_file)

    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    LOG_FORMAT = '%(asctime)s - %(levelname)s - %(message)s'
    formatter = logging.Formatter(LOG_FORMAT)
    fh = logging.FileHandler(log_file, encoding='utf-8')
    fh.setLevel(logging.INFO)
    fh.setFormatter(formatter)
    logger.addHandler(fh)

    #console打印
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.INFO)
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    #开启日志线程
    log_thread.start()

def Log_Info(text):
    thread_queque.put([logging.info, text])
   #logging.info(text)

def Log_Error(text):
    thread_queque.put([logging.error, text])
    #logging.error(text)

def Log_Close():
    thread_queque.put(None)
    log_thread.join()