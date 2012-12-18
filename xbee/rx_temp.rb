require 'rubygems'
require 'serialport'

# Serial Port Params
port_str = "/dev/tty.usbserial-A40081gS"
baud_rate = 9600
data_bits = 8
stop_bits = 1
parity = SerialPort::NONE
sp = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)

# Voltage Divider Params
vunit =  3.3 / 1024
vin = 3.3
r1 = 9770

# Thermistor Params
a = 0.003354016
b = 0.0002569850
c = 0.000002620131
d = 0.00000006383091
rref = 10000

def ln(x)
  return Math::log(x) / Math::log(Math::E)
end

def round(num, x)
  (num * 10**x).round.to_f / 10**x
end

def get_packet(sp)
  index = -1
  length = 0
  sum = 0
  while (c = sp.getc) do
    # wait for start
    if(c == 0x7e)
      puts
      packet = Array.new
      index = 0
      next
    end
  
    if(index == 0)
      length = c << 8
      index = 1
    elsif(index == 1)
      length = length | c
      index = 2
    elsif(index > 1 && index <= length + 1)
      packet.push(c)
      sum += c
      index = index + 1
    elsif(index > length + 1)
      index = -1
      length = 0
      packet.push(c)
      sum = 0xff - (sum & 0xff)
      if(packet.last != sum)
        packet = nil
      end
      return packet
    end
  end
end

while(1==1)
  packet = get_packet(sp)
  if(packet)
    puts Time.now
    offset = 0
    frame_type = packet[offset]
    printf "frame type: 0x%02x\n", frame_type
    offset = offset + 1
    
    source_addr_16 = 0
    2.times do
      source_addr_16 = source_addr_16 << 8
      source_addr_16 = source_addr_16 | packet[offset]
      offset = offset + 1
    end
    printf "source address: 0x%02x\n", source_addr_16
    
    printf "rssi: 0x%02x\n", packet[offset]
    offset = offset + 1
    
    printf "receive options: 0x%02x\n", packet[offset]
    offset = offset + 1
    
    printf "samples: 0x%02x\n", packet[offset]
    offset = offset + 1
    
    printf "analog mask: 0x%02x\n", packet[offset]
    offset = offset + 1
    
    printf "digital mask: 0x%02x\n", packet[offset]
    offset = offset + 1
    
    analog_value = 0
    2.times do
      analog_value = analog_value << 8
      analog_value = analog_value | packet[offset]
      offset = offset + 1
    end
    printf "analog value: %f volts\n", analog_value * vunit
    

    vt = analog_value * vunit
    rt = (vin*r1 - vt*r1) / vt

    printf "resistance: %f ohms\n", rt

    e = ln(rt/rref)
    t = 1 / (a + b * e + c * e ** 2 + d * e ** 3) - 273.15
    
    printf "temperature (C): %.2f\n", t
    printf "temperature (F): %.2f\n", t * 9/5 + 32
  else
    puts "malformed packet!"
  end
end
