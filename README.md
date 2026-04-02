# File Explorer
A simple cross-platform file explorer with searching, sorting, file creation and renaming.
Uses my own GUI library that uses SFML.

## Example
![](https://i.imgur.com/6sFDg0P.gif)

## Download
Download a .zip in releases, run the .exe in the bin folder.

## Build
### 1. Clone the repository  
```bash  
git clone https://github.com/Soawii/File-Explorer.git
```  
### 2. Go into the repository folder and build using cmake  
 ```bash  
 cd File-Explorer  
 mkdir build  
 cd build  
 cmake ..  
 ```  
### 3. Build the solution  
#### Windows:  
Open and build the generated .sln file with Release/Debug mode enabled.    
#### Linux:  
```bash  
make  
```  
### 4. Run the resulting file  
#### Windows:
```bash
cd build/FileExplorer/bin/
```
And run the .exe
#### Linux:  
```bash
cd FileExplorer/bin/
./FileExplorer  
```  
