cp FV/FvRecovery.fd YeeLoong.bin
tftp << EOF > /dev/null
connect kontais0
bin
put YeeLoong.bin
quit
EOF
