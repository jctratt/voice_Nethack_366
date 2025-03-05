#\!/bin/bash
for file in $(grep -l "delay_output(" *); do
  sed -i "s/delay_output(/delay_output(/g" "$file"
done
