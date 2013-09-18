#!/usr/bin/env python
#============================================================================
#
#                   E A S Y    G E R R I T    P U S H
#
# This script provide easy push command for Gerrit. It greatly relies on the
# correctly setting of username & user-email, and the git repo should be on
# correctly branch.
#
# Edit history
# ------------
#
# when          who    what, why, how
# ----------    ---    -----------------------------------------------------
# 2013-08-27    xdw    Create file
#
#
#============================================================================

import sys
import os
import re

def usage():
    print('''
Usage:
------
python gerrit_push.py
or
./gerrit_push.py
    ''')

def user():
    p = os.popen('git config user.name')
    return p.read().strip()

def email():
    p = os.popen('git config user.email')
    return p.read().strip()

def username():
    p = os.popen('git config user.email')
    m = re.search('.*(?=@)', p.read().strip())
    return m.group(0)

def repo():
    p = os.popen('git config remote.origin.url')
    res = p.read().strip()
    url = res.split(':')[1].strip()
    return url

def branch():
    p = os.popen('git branch')
    res = p.read().strip().split('\n')
    for l in res:
        if l.startswith('*'):
            return l.split('*')[1].strip()
    else:
        return None

def do_action(cmd):
    os.system(cmd)

if __name__ == '__main__':
    if len(sys.argv) != 1:
        usage()
        sys.exit(2)

    print('%30s' %(30*'-'))
    print('%10s  :   %s' %('User', user()))
    print('%10s  :   %s' %('Email', email()))
    print('%10s  :   %s' %('Repository', repo()))
    print('%10s  :   %s' %('Branch', branch()))
    print('%30s' %(30*'-'))

    cmd = '''git push ssh://%(username)s@10.128.161.91:29418/%(repo)s HEAD:refs/for/%(branch)s''' \
            %{'username':username(), 'repo': repo(), 'branch':branch()}
    print(cmd)
    #sys.exit(0)

    choice = raw_input('Confirm? [Y/n]')
    if len(choice) == 0:
        choice = 'Y'
    elif choice == 'y' or choice == 'Y':
        choice = 'Y'
    else:
        choice = 'N'

    if choice == 'N':
        print('Aborted!')
        sys.exit(0)

    do_action(cmd)
