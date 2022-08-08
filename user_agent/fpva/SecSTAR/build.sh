NEW_OUPUT="html-$(date +"%FT%H%M")"
cp -R html $NEW_OUPUT
python3 parser/SecSTAR.py --input_dir traces --output_dir "$NEW_OUPUT/images"