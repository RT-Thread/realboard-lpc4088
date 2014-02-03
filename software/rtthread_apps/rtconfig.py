import os

# RT-Thread root directory
RTT_ROOT = r'Z:\github\rt-thread'

# toolchains
EXEC_PATH = r'C:\Program Files (x86)\GNU Tools ARM Embedded\4.6 2012q2\bin'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
CXX = PREFIX + 'g++'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'mo'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'

DEVICE = ' -mcpu=cortex-m4'
CFLAGS = DEVICE + ' -mthumb -mlong-calls -O0 -fPIC -fno-exceptions'
AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp'
LFLAGS = DEVICE + ' -mthumb -Wl,-z,max-page-size=0x4 -shared -fPIC -e main -nostdlib'

CPATH = ''
LPATH = ''
