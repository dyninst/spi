source config

echo "=== Plotting ==="
if ls /tmp/*.xml &> /dev/null; then
		rm -f data/*.xml
fi

echo "Collecting data from $SERVER ..."
scp $SERVER:/tmp/*.xml data/ &> /dev/null
ssh $SERVER "rm -f /tmp/*.xml" &> /dev/null

echo "Collecting data from $CLIENT ..."
mv /tmp/*.xml data/
echo "Plotting ..."
cd ../SecSTAR
python SecSTAR.py
echo "Copying to web server ..."
mkdir -p $WEBSERVER
rm -rf $WEBSERVER/*
cp -rf ../html/* $WEBSERVER/
