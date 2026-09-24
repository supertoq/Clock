<img src="packaging/128px.png" height="128">

# Clock
A simple clock that uses Cairo to enable seamless scaling while the clock continues running.   
   
![clock main window](data/img/clock_preview_img_light2.png?raw=true) 
![clock main window](data/img/clock_preview_img_light1.png?raw=true) 

  
The main function of this app is simply to display the time, while allowing you to resize the window as desired.   
This is a feasibility test that I like.
  
  
## Installing  
The quickest way to install supertoq's Clock is to download the application from the [Releases](https://github.com/supertoq/Clock/releases) page.  
Installation proceeds as follows:  
```
cd ~/Downloads  
```  
```
flatpak install -y --user io.github.supertoq.clock.flatpak  
```  
  
You can also build the application yourself from the source code. One way to do this is using Flatpak Builder.

   
## Preparing To Build From Source 
 
### Install Flatpak Builder: 
```
flatpak install -y flathub org.flatpak.Builder 
```   

### Add The Flathub Repository: 
```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo 
```  
  
### Install The GNOME SDK: 
```
flatpak install org.gnome.Sdk/x86_64/49
```  
  
## Building The Application 
  
### Clone The OLED-Saver Repository:  
```
git clone https://github.com/supertoq/clock.git 
```  
### Build And Install The Application:  
```
cd clock
```  
```
flatpak run org.flatpak.Builder --user --install --force-clean _build-dir io.github.supertoq.clock.yml 
```  
  
### Running The Application  
```
flatpak run io.github.supertoq.clock 
```  
  
### Uninstalling:  
```
flatpak uninstall -y io.github.supertoq.clock 
```  

  
> [!Note]  
> Use of this code and running the application is at your own risk. I accept no liability.
  

