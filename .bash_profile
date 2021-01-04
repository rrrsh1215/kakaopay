# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
	. ~/.bashrc
fi

# User specific environment and startup programs

set -o vi
PS1="[\u \$PWD] "

alias  ll='ls -l'
alias  la='ls -al'
alias lrt='ls -lrt'

alias  lc='ls -al *.c'
alias  lh='ls -al *.h'
alias  lo='ls -al *.o'
alias lpc='ls -al *.pc'
alias lsh='ls -al *.sh'

alias    shl='cd $SH_HOME/shl'
alias    lib='cd $SH_HOME/lib'
alias libsrc='cd $SH_HOME/lib/src'
alias libhdr='cd $SH_HOME/lib/hdr'
alias libbin='cd $SH_HOME/lib/bin'
alias    inc='cd $SH_HOME/inc'
alias    src='cd $SH_HOME/src'
alias    bin='cd $SH_HOME/bin'
alias    sam='cd $SH_HOME/sam'
alias    cfg='cd $SH_HOME/cfg'
alias   json='cd $SH_HOME/jsonfile'
alias     gz='cd $SH_HOME/gz'
alias   open='cd $SH_HOME/opensource'

export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.272.b10-3.el8_3.x86_64
export PATH=$JAVA_HOME/bin:$SH_HOME/bin:$SH_HOME/shl:.:$PATH

#################################3
export SH_HOME=~
#################################3
