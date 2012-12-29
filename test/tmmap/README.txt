-G0 -mno-abicalls -fno-pic 

-fpic   编译时使用j，在链接时则是b


400810: 08100214  j 400850 <PeiImageRead+0x90>

j指令，后28bit是固定的，只有前4bit是相对PC的


