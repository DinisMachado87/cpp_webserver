let SessionLoad = 1
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd ~/CODE/my_webserv
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
let s:shortmess_save = &shortmess
set shortmess+=aoO
badd +1 ~/CODE/my_webserv
badd +1 srcs/sockets/ASocket.cpp
badd +39 srcs/logger/Logger.hpp
badd +87 srcs/logger/Logger.cpp
badd +12 srcs/webServ.hpp
badd +20 srcs/utils/Clock.hpp
badd +16 srcs/utils/Clock.cpp
badd +3151 term://~/CODE/my_webserv//156914:/bin/bash
badd +21 Makefile
badd +98 srcs/engine/Engine.cpp
badd +55 srcs/sockets/Listening.cpp
badd +1 srcs/sockets/Listening.hpp
badd +356 term://~/CODE/my_webserv//178459:/bin/bash
badd +125 srcs/server/Server.cpp
badd +0 all
argglobal
%argdel
$argadd ~/CODE/my_webserv
set stal=2
tabnew +setlocal\ bufhidden=wipe
tabrewind
edit srcs/sockets/Listening.cpp
let s:save_splitbelow = &splitbelow
let s:save_splitright = &splitright
set splitbelow splitright
wincmd _ | wincmd |
vsplit
wincmd _ | wincmd |
vsplit
2wincmd h
wincmd w
wincmd w
let &splitbelow = s:save_splitbelow
let &splitright = s:save_splitright
wincmd t
let s:save_winminheight = &winminheight
let s:save_winminwidth = &winminwidth
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe 'vert 1resize ' . ((&columns * 94 + 142) / 285)
exe 'vert 2resize ' . ((&columns * 95 + 142) / 285)
exe 'vert 3resize ' . ((&columns * 94 + 142) / 285)
argglobal
if bufexists(fnamemodify("term://~/CODE/my_webserv//156914:/bin/bash", ":p")) | buffer term://~/CODE/my_webserv//156914:/bin/bash | else | edit term://~/CODE/my_webserv//156914:/bin/bash | endif
if &buftype ==# 'terminal'
  silent file term://~/CODE/my_webserv//156914:/bin/bash
endif
balt srcs/logger/Logger.cpp
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
let s:l = 3151 - ((46 * winheight(0) + 24) / 49)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 3151
normal! 0
lcd ~/CODE/my_webserv
wincmd w
argglobal
balt ~/CODE/my_webserv/all
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 55 - ((25 * winheight(0) + 24) / 49)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 55
normal! 0
lcd ~/CODE/my_webserv
wincmd w
argglobal
if bufexists(fnamemodify("~/CODE/my_webserv/srcs/logger/Logger.cpp", ":p")) | buffer ~/CODE/my_webserv/srcs/logger/Logger.cpp | else | edit ~/CODE/my_webserv/srcs/logger/Logger.cpp | endif
if &buftype ==# 'terminal'
  silent file ~/CODE/my_webserv/srcs/logger/Logger.cpp
endif
balt ~/CODE/my_webserv/srcs/logger/Logger.hpp
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 84 - ((39 * winheight(0) + 24) / 49)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 84
normal! 034|
lcd ~/CODE/my_webserv
wincmd w
3wincmd w
exe 'vert 1resize ' . ((&columns * 94 + 142) / 285)
exe 'vert 2resize ' . ((&columns * 95 + 142) / 285)
exe 'vert 3resize ' . ((&columns * 94 + 142) / 285)
tabnext
edit ~/CODE/my_webserv/srcs/sockets/Listening.cpp
let s:save_splitbelow = &splitbelow
let s:save_splitright = &splitright
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
let &splitbelow = s:save_splitbelow
let &splitright = s:save_splitright
wincmd t
let s:save_winminheight = &winminheight
let s:save_winminwidth = &winminwidth
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe 'vert 1resize ' . ((&columns * 142 + 142) / 285)
exe 'vert 2resize ' . ((&columns * 142 + 142) / 285)
argglobal
if bufexists(fnamemodify("term://~/CODE/my_webserv//178459:/bin/bash", ":p")) | buffer term://~/CODE/my_webserv//178459:/bin/bash | else | edit term://~/CODE/my_webserv//178459:/bin/bash | endif
if &buftype ==# 'terminal'
  silent file term://~/CODE/my_webserv//178459:/bin/bash
endif
balt ~/CODE/my_webserv/srcs/sockets/Listening.cpp
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
let s:l = 356 - ((48 * winheight(0) + 24) / 49)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 356
normal! 034|
lcd ~/CODE/my_webserv
wincmd w
argglobal
balt ~/CODE/my_webserv/Makefile
setlocal foldmethod=manual
setlocal foldexpr=0
setlocal foldmarker={{{,}}}
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldenable
silent! normal! zE
let &fdl = &fdl
let s:l = 52 - ((37 * winheight(0) + 24) / 49)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 52
normal! 069|
lcd ~/CODE/my_webserv
wincmd w
exe 'vert 1resize ' . ((&columns * 142 + 142) / 285)
exe 'vert 2resize ' . ((&columns * 142 + 142) / 285)
tabnext 1
set stal=1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0 && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20
let &shortmess = s:shortmess_save
let &winminheight = s:save_winminheight
let &winminwidth = s:save_winminwidth
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
set hlsearch
nohlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
