import os
cwd = os.path.split(os.path.realpath(__file__))[0]

# RT-Thread root directory
RTT_ROOT = r'Z:\github\rt-thread'
BSP_ROOT = cwd + '/../rtthread_examples/examples/realboard'
# toolchains
EXEC_PATH = r'C:\Program Files (x86)\GNU Tools ARM Embedded\4.6 2012q2\bin'

if os.getenv('RTT_ROOT'): RTT_ROOT = os.getenv('RTT_ROOT')
if os.getenv('BSP_ROOT'): BSP_ROOT = os.getenv('BSP_ROOT')
if os.getenv('RTT_EXEC_PATH'): EXEC_PATH = os.getenv('RTT_EXEC_PATH')

PLATFORM = 'gcc'
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
