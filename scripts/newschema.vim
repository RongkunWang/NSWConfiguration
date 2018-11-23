" This script corrects the configuration files after the schema change.
" To run it, copy it to same directory with configuration file,
" open a configuration file with vim, then type the following
" :so newschema.vim
" After this you can save and close the file with pressing zz
" 
" Alternatively, you can use the second script directly while opening a file
" vim myconfig_file.json -s newschema_keys.vim
" After this you can press <ESC>, then save and close the file with pressing zz

call setreg('m', "/reg076\<cr>V/reg077\<cr>:s/_0\"/_1\"/g\<cr>/reg077\<cr>V/reg078\<cr>:s/_0\"/_2\"/g\<cr>/reg078\<cr>V/reg079\<cr>:s/_0\"/_3\"/g\<cr>")
exe "norm! @m"

call setreg('m', "/reg092\<cr>V/reg093\<cr>:s/_0\"/_1\"/g\<cr>/reg093\<cr>V/reg094\<cr>:s/_0\"/_2\"/g\<cr>/reg094\<cr>V/reg095\<cr>:s/_0\"/_3\"/g\<cr>")
exe "norm! @m"

call setreg('m', "/reg108\<cr>V/reg109\<cr>:s/_0\"/_1\"/g\<cr>/reg109\<cr>V/reg110\<cr>:s/_0\"/_2\"/g\<cr>/reg110\<cr>V/reg111\<cr>:s/_0\"/_3\"/g\<cr>")
exe "norm! @m"

call setreg('m', "/reg116\<cr>V/reg117\<cr>:s/_0/_1/g\<cr>/reg117\<cr>V/reg118\<cr>:s/_0/_2/g\<cr>/reg119\<cr>V/reg120\<cr>:s/_0/_2/g\<cr>")
exe "norm! @m"

call setreg('m', ":%s/0:3]/3:0]\<cr>")
exe "norm! @m"

call setreg('m', ":/reg006eopAndNullEventEnable\<cr>/sroc\<cr>nllllr2")
exe "norm! @m"

call setreg('m', ":%s/11_8/[11:8]\<cr>")
exe "norm! @m"

call setreg('m', ":%s/7_0/[7:0]\<cr>")
exe "norm! @m"

call setreg('m', ":%s/10_7/[10:8]\<cr>")
exe "norm! @m"

call setreg('m', ":%s/8_0/[7:0]\<cr>")
exe "norm! @m"

call setreg('m', "/sL0enaV/\<cr>o\"slh\": 0,\<cr>\"slxh\": 0,\<cr>\"stgc\": 0,")
exe "norm! @m"
