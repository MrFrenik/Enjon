rm -rf Build && mkdir Build && cd Build && cmake ../. -DCMAKE_BUILD_TYPE=Release && cd ..
cd Editor && rm -rf Build && mkdir Build && cd Build && cmake ../. -DCMAKE_BUILD_TYPE=Release && cd .. && cd .. 
bash ./Proc/OSX/compile.sh
bash ./Proc/OSX/run.sh