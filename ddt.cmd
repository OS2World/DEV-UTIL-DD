/*

*/
/*trace ?r                                       
*/                                               
/*
'dd ddd dddd i /T7'
*/
ddT.1 = 'dd /fddd.ddd dd t1 && type ddT.ddD ddT.ddt'
ddT.2 = 'dd /fddd.ddd dd t2 && type ddT.ddD ddT.ddt'
ddT.3 = 'dd /fddd.ddd dd t3 && type ddT.ddD ddT.ddt'
ddT.4 = 'dd /fddd.ddd dd t4 && type ddT.ddD ddT.ddt'
ddT.5 = 'dd /fddd.ddd dd t5 && type ddT.ddD ddT.ddt'
ddT.6 = 'dd /fddd.ddd dd t6 && type ddT.ddD ddT.ddt'
ddT.7 = 'dd /fddd.ddd dd t7 && type ddT.ddD ddT.ddt'
ddT.7 = 'dd /fddd.ddd dd t8 && type ddT.ddD ddT.ddt'

                                                 
dd.1  = 'copy dd.Exe ddd.exe && dd ddd dd i /c dd '
dd.2  = 'dd dd i /c dd                            '        
dd.3  = 'dd i /c dd*.Exe                          '        
dd.4  = 'dd i /b /mapfile:dd.map ddd.exe          '
dd.5  = 'dd i /c dd*.Exe                          '
dd.6  = 'dd ddd dd i /M:0xM0x0010000 dd i         '
dd.7  = 'dd.exe dd.exe I dd.exe                   '        
dd.8  = 'dd dd I dd                               '        
dd.9  = 'dd I dd.exe                              '        
dd.10 = 'dd I /I dd*.exe                          '        
dd.11 = 'dd I /B dd.Exe                           '
dd.12 = 'dd I /MAPFILE:other.map dd.dll           '
dd.13 = 'dd I /M:0x10032 dd*.exe                  '
dd.14 = 'dd I /B /D /DDEBUG your.exe              '
dd.15 = 'dd i.exe                                 '

parse arg T


if dd.T <> 'DD.'T then
   call ddTest(dd.T)
else do 
   say 'Performing ddT'

   T = 1
   do while dd.T <> 'DD.'T
      call ddTest(''dd.T'')
      T = T + 1
   end

   T = 1
   do while ddT.T <> 'DDT.'T
      call ddTest(''ddT.T'')
      T = T + 1
   end 
end 

return rc


ddTest:
say
say '@'arg(1)
'@'arg(1)

if rc <> 0 then do
  say 'rc = 'rc' on ddT @'arg(1)
  txt = linein()
end
return rc

