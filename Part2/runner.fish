#!/bin/fish

set packetSizes 40 44 48 52 60 250 300 552 576 628 1420 1500

echo "Building code..."

./waf build

echo "Removing previous output..."

echo "TCP Type = Vegas"

rm -rf "*_*.xml"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=vegas -ps=$x" 
end

echo "TCP Type = Veno"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=veno -ps=$x" 
end

echo "TCP Type = WestWood"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=westwood -ps=$x" 
end
