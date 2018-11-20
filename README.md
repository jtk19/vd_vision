Veed Machine Vision Library  
----------------------------  
    
COMPILING & RUNNING  
--------------------------------  
  
THE CONFIG HEADER SOURCE FILE  
------------------------------  
  
Before compiling the repository there are 3 important compilations switches defined in  
  
veed-studio-cv/veed/src/common/config.h  
  
Most of the imporatnt directories and definitions are defined in this "config.h" file.  So 
it is here one should look for definitions.  
  
The 3 important compilation switches are:  
  
#define DISPLAY_VIDEO_OPTICALFLOW  
//#define DISPLAY_VIDEO_ORIENT90		// Un-comment this to display video within orient90.  
#define ORIENT90_SINGLE_FILE_RUN		// Run orient90 for a single video file. Comment out for testing only.  
  
The 2 display switches are to display video as the processing happens in windows in 
opticalflow and orient90 respectively.   
* It is important to comment out both these switches on Amazon AWS before compiling every time 
because display to a screen is not avalible on the cloud.  
* On a PC, I turn display off for orient90 and turn display on for opticalflow.   
  
When the ORIENT90_SINGLE_FILE_RUN is turned on orient90 will process only a single video.  
This switch is turned off only for testing and then orient90 will process all the video file
defined in the veed.cfg config file.

* The other important thing in common/config.h is below:  
  
enum Veed_Orient_T  
{  
	VEED_ORIENT_0 = 0,			// original frame with no turning  
	VEED_ORIENT_90 = 1,			// needs 90 degree clockwise rotation, 1  
	VEED_ORIENT_180 = 2,		// needs 180 degree clockwise rotation, 2  
	VEED_ORIENT_270	= 3			// needs 270 degree clockwise rotation, 3  
};  
orient90 will output results as 0 or 1 or 2 or 3, denoting by how many degrees the video needs to be turned as above.    
Results will also be output to "/home/data/veed/detect/xxx.det" file which is read in and used by opticalflow program.    
  
COMPILATION  
-------------  
  
* Change to the root source directory: $ cd <..install directory..>/veed-studio-cv/veed/src/
  
* Remove previous compilations with:  $ make clean  
    
* Compile everything with: $ make   
All the executables and configs will appear in veed-studio-cv/veed/src/build/ directory  
  
* Install the required files with: $ make install  
The executables and configs required for running the programs will appear in...  
  
<..install directory..>/veed-studio-cv/veed/src/dist  
$(HOME)/veed/  
  
* The final files are:  
opticalflow*  orient90*  veed.cfg  veed_orient90*  
    
* To run orient90 for a single video run:  $ ./orient90 <subdir> <video filename>    
e.g: $ ./orient90 "90" "10.avi"  
The above will run orient90 for "/home/data/veed/video/90/10.avi"  

* To run orient90 for a batch of files do the following:   
- Create sub-directories in /home/data/veed/video/ input directory and put the videos in the sub-directories.   
- Open "veed.cfg" file in an editor.  You will see 2 valiable-value pairs in 2 lines.
In veed.cfg, lines with # as the first character are ignored as comments.  
  
#veed_video_subdir=90,270,phase1  
veed_video_subdir=90,270,client1  
veed_video_extensions=avi,mp4,MOV,mov  
  
The program will run for all the video files with the extensions defined in "veed_video_extensions"  
in all the sub-directories defined in "veed_video_subdir".  For example, the program for the above "veed.cfg"
will run for all video files with extensions .avi, .mp4, .MOV and .mov in directories:  
/home/data/veed/video/90/  
/home/data/veed/video/270/  
/home/data/veed/video/client1/  
  
- Now, to run for all videos with multi-processing, run:  $ ./veed-orient90
  
* It is recommended that, for processing multiple videos, oreint90 be run for all the videos in 
multi-processing mode by running veed-oreint90 as above first.  
  
* Now run opticalflow:  $ ./opticalflow  
opticalflow will run for all videos in sub-directories defined as above in the "veed.cfg" config file.  
 
* The results will be written into /home/data/veed/write/ diectory under <subdir>_<filename>.ext.  
For example, for /home/data/veed/video/90/10.mp4  the results will be written as:  
  
/home/data/veed/write/90_10.mp4  		<-- The cropped, turned and edited final video file  
/home/data/veed/write/90_10_marked.mp4  <-- The above video with opticalflow centres and face & body marked    
/home/data/veed/write/90_10.json  <-- The JSON file giving the results  
  
There is a sample JSON file: veed-studio-cv/veed/doc/sample.json
  
  


INSTALLATION  
--------------------------------

(1) Operating System: 

Preferred: Install the latest 64 bit Ubuntu version running both NVIDIA CUDA and Amazon AWS platform. 
A good alternative is the latest 64 bit Fedora running both CUDA and Amazon AWS.  
  
  
(2) Install Cambridge University's math and machine vision libraries, extracting the latest version
from respective Github repositories. 
  
Toon : Math library  
libCVD: Machine Vision Library  
  
[https://www.edwardrosten.com/cvd/](https://www.edwardrosten.com/cvd/)

The installation is straightforward. Clone from Github to a local directory, configure, build and install.
Then copy the built directory to /usr/local/opt/  
  
$ git clone <Github repo URL>  
$ cd <Toon|libcvd>  
$ ./configure  
$ make -j4  
$ sudo make install  
$ sudo mkdir /usr/local/opt  
$ sudo chown -R <ubuntu|other installation username> /usr/local/opt  
$ cp -r ./<Toon|libcvd> /usr/local/opt  
   
  
(3) Build and install the latest version of OpenCV.  
  
Before installation, please be sure to install all the dependencies required outlined in this Docker
installation.  
[LibCVD Dependencies](https://github.com/StevenPuttemans/docker/blob/master/Dockerfile_basic)  
  
$ sudo su  
#  apt update -y  
#  apt upgrade -y  
#  apt install -y git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy python3-dev python3-numpy python-scipy python3-scipy 
libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev libeigen2-dev libeigen3-dev "ffmpeg*" "gstreamer*" g++-7 cmake  
#   
#  exit  
$ alias g++='g++-7 -std=c++11'  
$ git clone https://github.com/opencv/opencv.git  
$ cd opencv  
$ mkdir build  
$ cd build  
$ make -DWITH_TBB=ON -DCUDA_ARCH_BIN=6.1 -DCUDA_ARCH_PTX=6.1 -DENABLE_IMPL_COLLECTION=ON -DTBB_ENV_INCLUDE=/usr/include -DTBB_ENV_LIB=/usr/lib/x86_64-linux-gnu/libtbb.so -DTBB_VER_FILE=/usr/include/tbb/tbb_stddef.h ..
_(In the step above, please set CUDA switches to the correct latest version)_  
$ make -j6  
$ sudo make install  
$ cd ../..  
$ sudo mkdir /usr/local/opt
$ sudo chown -R $USER /usr/local/opt
$ sudo cp -r opencv /usr/local/opt  
  
  
  
(4) Now we are ready to create the structure required for our codebase:  
$ sudo su  
# mkdir -p /home/data/veed/video        // This is our video input directory. 
# chown -R $USER /home/data
# exit  
To process videos with our program, create subdirectories within this video directory   
and put .avi video files in each sub-directory  
e.g:  
$ cd /home/data/veed/video  
$ mkdir 0 90 180 270        
Four directories created above named as above.  
Copy your .avi video files into one of the above sub-directories.  

# sudo su
# mkdir -p /home/data/veed/detect
# mkdir -p /home/data/veed/write        // This is the directory to which we write our processed videos.  
# chown -R <ubuntu|other installation user> /home/data  
# exit

* Remember to set LD_LIBRARY_PATH and INCLUDE_PATH by adding this to your .bashrc file
$ echo export LD_LIBRARY_PATH=.:/usr/lib64:/usr/local/lib64:/usr/local/lib >> ~/.bashrc
$ echo export INCLUDEPATH=.:/usr/include:/usr/local/include >> ~/.bashrc
$ source ~/.bashrc


(5) Now we are ready to install our 			
$ cd  
$ mkdir src  
$ cd src  
$ git clone https://<butbucket uid>@bitbucket.org/timsabba/veed-studio-cv.git  
$ alias v='cd /home/ubuntu/src/veed-studio-cv/veed/src'  
$ v  
$ make clean  
$ make  
$ make install  
  
The executables will be installed to _$(HOME)/veed/_ directory  
  $ cd ~/veed  
  $ ls  
orient90*  orient90.cfg  veed_orient90*  
  
*To run our program for a single .avi file* in our input video direcory, run "orient90"
with the name of the sub-directory as the first parameter and 
the name of the video file as the second parameter.   
  
$ ./orient90 "90" "7.avi"
  
The above will run our program for a single video file:  
/home/data/veed/video/90/7.avi  
  
and write the result to:  
/home/data/veed/write/90_7.avi  
  
  
*To run our program for multiple .avi files with multi-processing,* edit the config file 
and enter all the sub-directores which have videos to process:  
  
$ gedit ./veed.cfg &  
  
veed_video_subdir=90,270  
  
Enter all the sub-directories comma separated for variable _veed_video_subdir_.
Then run the multi-processing executable.  
  
$ ./veed_orient90  
  
This will run our program for all the .avi video files in:  
/home/data/veed/video/90/   
/home/data/veed/video/270/  
  
and write the results into our write directory as...   
/home/data/veed/write/90_*.avi  
/home/data/veed/write/270_*.avi  


  


 





