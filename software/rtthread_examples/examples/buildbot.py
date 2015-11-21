import os
import sys

examples_dir = '.'

target = ''
if len(sys.argv) == 2:
    target = sys.argv[1]

projects = os.listdir(examples_dir)
for item in projects:
    project_dir = os.path.join(examples_dir, item)
    if os.path.isfile(os.path.join(project_dir, 'SConstruct')):
        command = 'scons ' + target
        command = command + ' --directory=' + project_dir

        while True:
            print 'Building in directory: ', item
            result = os.system(command)

            if result != 0:
                print 'build failed @ %s !!' % project_dir
                cmd = raw_input("buildbot>>")
                if cmd == 'continue':
                    continue
                elif cmd == 'skip':
                    break
                elif cmd == 'exit':
                    exit (0)
                elif cmd == 'help':
                    print 'continue, exit'
                else:
                    print 'unknown command!'
            else:
                break;
