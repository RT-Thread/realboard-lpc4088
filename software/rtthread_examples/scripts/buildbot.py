import os
import sys

examples_dir = os.path.join('software', 'rtthread_examples', 'examples')

projects = os.listdir(examples_dir)
for item in projects:
    project_dir = os.path.join(examples_dir, item)
    if os.path.isfile(os.path.join(project_dir, 'SConstruct')):
        if os.system('scons --directory=' + project_dir) != 0:
            print 'build failed!!'
            break
