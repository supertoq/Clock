<img src="packaging/io.github.supertoq.clock.svg" height="128">

# Clock
A simple desktop clock  
  
![clock main window](data/img/clock_preview_img_light1.png?raw=true) 
  
![clock main window](data/img/clock_preview_img_light2.png?raw=true) 
  
The main function is to display the time, which can be continuously scaled to any size.  
  
  
## Installing:  
The quickest way to install supertoq's Clock is to download the application from the [Releases](https://github.com/supertoq/Clock/releases) page.  
Installation proceeds as follows:  
```
cd ~/Downloads  
```  
```
flatpak install -y --user io.github.supertoq.clock.flatpak  
```  
  
You can also build the application yourself from the source code. One way to do this is using Flatpak Builder.
  
## Building with Flatpak Builder.  

### Dependencies:
  
#### Ubuntu/Debian  
```
sudo apt update && sudo apt install flatpak flatpak-builder
```  
  
#### Fedora  
```
sudo dnf upgrade && sudo dnf install flatpak flatpak-builder 
```  
  
#### Arch
```
sudo pacman -Syu && sudo pacman -S flatpak flatpak-builder 
```  

### Add Flathub Repository: 
```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo 
```  
  
### Install the GNOME SDK 49: 
```
flatpak install org.gnome.Sdk/x86_64/49
```  
  
## Building the Flatpak File:
  
### Clone Repository:  
```
git clone https://github.com/supertoq/Clock.git 
```  
  
```
cd Clock 
```  
```
flatpak-builder --user --install --force-clean _build-dir io.github.supertoq.clock.yml 
```  
  
### Running:  
```
flatpak run io.github.supertoq.clock 
```  
  
#### If you want to uninstall: 
```
flatpak uninstall --delete-data -y io.github.supertoq.clock 
```  
  
> [!Note]  
> Use of this code and running the application is at your own risk. I accept no liability.
  