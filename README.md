# Clock
A simple desktop clock  
  
![clock main window](data/img/clock_preview_img_light1.png?raw=true) 
  
![clock main window](data/img/clock_preview_img_light2.png?raw=true) 
  
The main function is to display the time, which can be continuously scaled to any size.  
  
  
## Installation:  
The quickest way to install supertoq's Clock is to download the application from the [Releases](https://github.com/supertoq/Clock/releases) page.  
Installation proceeds as follows:  
```
cd ~/Downloads  
```  
```
flatpak install -y --user io.github.supertoq.clock.flatpak  
```  
  
You can also build the application yourself from the transparent source code; here’s one way using Flatpak Builder.

  
## Building and Installing with Flatpak Builder.  

### Preparation and Dev Depentencies:
  
#### Ubuntu/Debian  
```
sudo apt update && sudo apt install flatpak flatpak-builder
```  
  
#### Fedora  
```
sudo dnf install flatpak flatpak-builder 
```  
  
#### Arch
```
sudo pacman -S flatpak flatpak-builder 
```  

### Add Flathub repository: 
```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo 
```  
  
### Within Flatpak, the `Gnome SDK 49` is required: 
```
flatpak install org.gnome.Sdk/x86_64/49
```  
  
## Install Clock:
  
### Clone repository:  
```
git clone https://github.com/supertoq/Clock.git 
```  
  
```
cd Clock 
```  
```
flatpak-builder --user --install --force-clean _build-dir io.github.supertoq.clock.yml 
```  
  
## Run Clock:  
```
flatpak run io.github.supertoq.clock 
```  
  
## If you want to uninstall Clock:  
```
flatpak uninstall --delete-data -y io.github.supertoq.clock 
```  
  
> [!Note]  
> This code is part of my learning project.  
Use of the code and execution of the application is at your own risk; 
I accept no liability!
  