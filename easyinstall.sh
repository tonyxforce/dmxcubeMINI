sudo apt install python3 python3-venv python-is-python3 nodejs npm

cd scripts
npm install
cd ..

curl https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py > get-platformio.py
python3 get-platformio.py
del get-platformio.py

%userprofile%\.platformio\penv\Scripts\pio run --target=upload --upload-port=2.0.0.1