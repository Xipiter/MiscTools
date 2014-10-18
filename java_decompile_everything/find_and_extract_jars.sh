find ./xtinst -name "*.jar" -exec cp {} ./jars \;
cd jars; find ./ -name "*.jar" -exec unzip {} \;
