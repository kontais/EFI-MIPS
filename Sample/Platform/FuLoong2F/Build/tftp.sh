cp FV/FvRecovery.fd FuLoong2F.bin
tftp << EOF > /dev/null
connect kontais0
bin
put FuLoong2F.bin
quit
EOF
