./run $1

display part_1.ppm &
display part_2.ppm &
display part_3.ppm &
display part_4.ppm &

read
kill `jobs -p`

