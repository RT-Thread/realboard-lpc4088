import os
import sys

examples_dir = os.path.join('software', 'rtthread_examples', 'examples')

target = ''
if len(sys.argv) == 2:
	target = sys.argv[1]
elif len(sys.argv) == 1:
    # build some common libraries
    path = os.path.join('software', 'rtthread_examples', 'examples', 'realboard')
    os.system('scons --buildlib=finsh --directory=path')
    os.system('scons --buildlib=RTGUI --directory=path')
    os.system('scons --buildlib=LwIP --director=path')

projects = os.listdir(examples_dir)
for item in projects:
    project_dir = os.path.join(examples_dir, item)
    if os.path.isfile(os.path.join(project_dir, 'SConstruct')):
    	command = 'scons ' + target
    	command = command + ' --directory=' + project_dir

    	print 'Building in Examples: ', item
        if os.system(command) != 0:
            print 'build failed!!'
            break
