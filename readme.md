```
 __         ______     ______    
/\ \       /\  __ \   /\  ___\   
\ \ \____  \ \  __ \  \ \ \____  
 \ \_____\  \ \_\ \_\  \ \_____\ 
  \/_____/   \/_/\/_/   \/_____/ 
                                 
 ______     ______               
/\  __ \   /\  ___\              
\ \ \_\ \  \ \___  \             
 \ \_____\  \/\_____\            
  \/_____/   \/_____/            
```
# Running the OS
To build the OS using the provided makefile, a linux enviroment is the required, be it native or wsl. 

It should be noted that `make start` and `make bochsdbg` will only work if called from wsl in a windows machine since they open the `.exe`s directly (you can just edit them and remove the `.exe` part on linux).