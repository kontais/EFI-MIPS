echo -off
set -v LOGGER echo

if not "%new_broadcast_address%x"  == "x" then
#  %LOGGER% New Broadcast Address: %new_broadcast_address%
  if not "%new_broadcast_address%x" == "^%new_broadcast_address^%x" then
     set -v new_broadcast_arg "broadcast %new_broadcast_address%"
  endif
endif

if not "%new_subnet_mask%x"  ==  "x" then
#  %LOGGER% New Subnet Mask for %interface%: %new_subnet_mask%
  if not "%new_subnet_mask%x" == "^%new_subnet_mask^%x" then
    set -v new_netmask_arg "netmask %new_subnet_mask%" 
  endif  
endif

if %reason% == FAIL then
  %LOGGER% DHCP Client startup failed
  goto exit_with_hooks
endif

if %reason%  ==  PREINIT then
  ifconfig %interface% inet 0.0.0.0 netmask 0.0.0.0 broadcast 255.255.255.255 up
  route add -host 255.255.255.255 -interface %interface%
  goto exit_with_hooks
endif

#
# BOUND and REBOOT cases are the same but we have no 'or' syntax in batch files
#
if %reason%  ==  BOUND then
  goto common_config
endif
if %reason%  ==  REBOOT then
  goto common_config
endif

goto exit_with_hooks

:common_config

if  not "%old_routers%x"  == "x" then
    if "%old_routers%x" == "^%old_routers^%x" then
      route delete default %old_ruters%
    endif   
endif



:skip

ifconfig %interface% inet %new_ip_address% %new_netmask_arg% %new_broadcast_arg%
route add default %new_routers% 
echo search %new_domain_name%  >a \etc\resolv.conf
echo nameserver %new_domain_name_servers%  >>a \etc\resolv.conf
set -v old_routers "%new_routers%"

:exit_with_hooks

set -d LOGGER

