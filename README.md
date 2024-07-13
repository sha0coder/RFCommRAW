# Install

```bash
sudo apt install libbluetooth-dev
pip install RFCommRAW
```

# Usage

```python
>>> RFCommRAW.interfaces()
[0]
>>> RFCommRAW.scan(0)
['A0:94:1A:87:50:E7']


for channel in range(0,0xffff):
    try:
        r = RFCommRAW.communicate('A0:94:1A:87:50:E7', channel, read_sz, timeout, b'AT\r\n') 
        # params:  bssid, channel, recv-size, bytes-to-send
        if r:
            print(f'channel {channel}: {r}')
    except:
        pass
```


