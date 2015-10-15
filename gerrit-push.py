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
# 2014-07-07    xsl    Do pull before push
#
#============================================================================

import sys
import os
import re
import platform
import commands
import shlex, subprocess

gerritserver='10.128.161.209:29418'

def usage():
    print('''
Usage:
------
python gerrit_push.py
python gerrit_push.py -t
or
./gerrit_push.py
./gerrit_push.py -t
    ''')

COLORS = {None     :-1,
          'normal' :-1,
          'black'  : 0,
          'red'    : 1,
          'green'  : 2,
          'yellow' : 3,
          'blue'   : 4,
          'magenta': 5,
          'cyan'   : 6,
          'white'  : 7}

ATTRS = {None     :-1,
         'bold'   : 1,
         'dim'    : 2,
         'ul'     : 4,
         'blink'  : 5,
         'reverse': 7}

RESET = "\033[m"

def is_color(s): return s in COLORS
def is_attr(s):  return s in ATTRS

def _Color(fg = None, bg = None, attr = None):
    fg = COLORS[fg]
    bg = COLORS[bg]
    attr = ATTRS[attr]

    if attr >= 0 or fg >= 0 or bg >= 0:
      need_sep = False
      code = "\033["

      if attr >= 0:
        code += chr(ord('0') + attr)
        need_sep = True

      if fg >= 0:
        if need_sep:
          code += ';'
        need_sep = True

        if fg < 8:
          code += '3%c' % (ord('0') + fg)
        else:
          code += '38;5;%d' % fg

      if bg >= 0:
        if need_sep:
          code += ';'
        need_sep = True

        if bg < 8:
          code += '4%c' % (ord('0') + bg)
        else:
          code += '48;5;%d' % bg
      code += 'm'
    else:
      code = ''
    return code

def colorer(on=True, fg=None, bg=None, attr=None):
    if on:
      c = _Color(fg, bg, attr)
      def f(fmt, *args):
        str = fmt % args
        return ''.join([c, str, RESET])
      return f
    else:
      def f(fmt, *args):
        return fmt % args
      return f


def get_list_select(candidates, prompt, color, cols=2):
    #if platform.system() == "Windows":
    #    fmt = "{0}: {1}"
    #    err_txt = u"Invalid Input, please enter again."
    #else:
    #    fmt = "\033[1;31;40m{0}\033[0m: {1}"
    #    err_txt = u"\033[1;31;40m Invalid Input, please enter again. \033[0m"
    err_txt = color(u"Invalid Input, please enter again.")
    for idx, cnd in enumerate(candidates, 1):
        #prompt += fmt.format(idx, cnd)
        prompt += color(str(idx)) + ":\t" + cnd
        if idx % cols == 0:
            prompt += "\n"
        else:
            prompt += "\t\t"
    if cols > 1:
        prompt += "\n"

    while True:
        try:
            enter = int(raw_input(prompt.encode(sys.stdout.encoding)).strip())
        except ValueError:
            continue
        if enter > 0  and enter <= len(candidates):
            enter -= 1
            return enter
        print err_txt
    return -1


def git_config(config):
    p = os.popen('git config ' + config)
    return p.read().strip()

def username():
    return git_config('user.name')

def branch_merge(branch):
    merge = git_config('branch.%s.merge' % branch)
    return merge.split('/')[-1].strip()

"""git config key can not contain "_" letter
"""
def gerrit_account():
    return git_config('user.gerrit-account')

def email():
    return git_config('user.email')

def emailuser():
    #email().split()[0]
    m = re.search('.*(?=@)', email())
    if m is None:
        return None
    return m.group(0)

def repo():
    res = git_config('remote.origin.url')
    url = res.split(':')[1].strip()
    return url

def branch():
    p = os.popen('git branch')
    res = p.read().strip().split('\n')
    for l in res:
        if l.startswith('*'):
            return l.split('*')[1].strip()
    raise Exception("None branch")
    return None

def get_cmd(c):
    print('The project some important information')
    print('%78s' %(78*'-'))
    print('%20s  :   %s' %('User', c(username())))
    print('%20s  :   %s' %('Email', c(email())))
    print('%20s  :   %s' %('Gerrit Account', c(gerrit_account())))
    print('%20s  :   %s' %('Repository', c(repo())))
    print('%20s  :   %s' %('Branch', c(branch())))
    print('%78s' %(78*'-'))

    #return ["git",
    #        "push",
    #        "ssh://%(user)s@172.24.61.94:29418/%(repo)s" % {'user':gerrit_account(), 'repo': repo()},
    #        "HEAD:refs/for/%(branch)s" % {'branch':branch_merge(branch()},
    #        ]
    #shlex.split can be used
    return "git push ssh://%(usr)s@%(srv)s/%(repo)s HEAD:refs/for/%(branch)s" \
            % {'usr':gerrit_account(), 'srv':gerritserver,
               'repo': repo(), 'branch':branch_merge(branch())}


def do_action(cmd, prompt, output=False):
    color_print(prompt)
    if output:
        #p = os.popen(cmd)
        #return p.read().strip()

        #commands is not work properly in MingGW.
        #return commands.getoutput(cmd)
        try:
            output = subprocess.check_output(shlex.split(cmd))
        except Exception as error:
            output = str(error)
            print "command is ", cmd
            print error
            return os.system(cmd)
        return output
    else:
        return os.system(cmd)
        #return subprocess.call(cmd)

if __name__ == '__main__':
    need_sync = True
    if len(sys.argv) == 2 and sys.argv[1] == "-t":
        need_sync = False
    elif len(sys.argv) != 1:
        usage()
        sys.exit(2)

    c = colorer(False if platform.system() == "Windows" else True ,
            'red', 'normal', 'bold')

    def color_print(fmt, *args):
        sys.stdout.write(c(fmt, *args))
        sys.stdout.flush()

    if username() is None or username() == "":
        print "Use command ",
        color_print(" 'git config user.name' ")
        print "to set your name first."
        sys.exit(3)

    if emailuser() is None:
        print "Use command",
        color_print(" 'git config user.email' ")
        print "to set your email. \nIt is the email address that you set in Gerrit."
        sys.exit(4)

    if len(gerrit_account()) == 0:
        color_print("no gerrit account.\n")
        #git config color.ui

        candidates = [username(), emailuser(), "Input Other"]
        #candidates = list([username(), emailuser()]) + ["Input Other"]
        select = get_list_select(candidates,
                "Please setting your gerrit account, which is always the eamil account.:\n",
                c)
        if select == -1 or select == (len(candidates) -1):
            gerritaccount = raw_input("Please enter your gerrit account:\n").strip()
        else:
            gerritaccount = candidates[select]
        os.popen(' '.join(['git', 'config', "user.gerrit-account",
            gerritaccount]))

    cmd = get_cmd(c)
    color_print(cmd)
    print("")

    choice = raw_input('Please Confirm it [Y/n] :')
    if len(choice) == 0 or choice.lower() == 'y' :
        if need_sync:
            msg=do_action("git pull --rebase",
                    "First Synchronize Server Status!\n")
            if msg != 0:
                sys.exit(msg)

        ret = do_action(cmd, "Now Push Commit to Gerrit\n", True)
        #print ret
        if isinstance(ret, str) and ret.startswith("Permission denied (publickey)."):
            color_print('''Please login 'http://gerrit.tcl-ta.com:8081/login/',
click user name on the right top corner, navigate 'Settings/SSH Public Keys',
add SSH Public Key which is the content of file '~/.ssh/id_rsa.pub'.\n''')
            sys.exit(-1)

        sys.exit(0)
    else:
        #print('Aborted!')
        print "You can edit 'gerrit-account' in '.git/config'"
        print "You can edit 'remote.origin.url' in '.git/config'"
        #print '''You can checkout branch which you want commit. Check command
#`git branch` first'''
        sys.exit(0)
