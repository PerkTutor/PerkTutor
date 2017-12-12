%7 /command "open sftp://%1:%2@%3" "get %4/%6 %5\%6" "exit"
:: Gets data from the remote server and copies to the local
:: Syncs data from the local to the remote server
:: 1 Username
:: 2 Password
:: 3 Server URL
:: 4 Remote directory
:: 5 Local directory
:: 6 File name
:: 7 FTP client executable