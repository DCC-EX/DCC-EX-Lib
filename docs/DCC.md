# DCC
The DCC layer provides functions for interacting with devices on the bus/track. Compared to the original DCC++ code, it does not perform string parsing operations and instead exposes an API with parameterized functions for each functionality in the original DCC++ code.

## API Documentation

### DCCMain and DCCService Classes
For more effecient memory usage and compartmentalization of features, DCC is split into two classes, one for the main track and one for the service track. The ```DCCMain``` class is capable of device control and railcom communications, given the correct hardware. The ```DCCService``` class is capable of service mode programming only, and cannot control trains.

See pages [DCCMain](DCCMain) and [DCCService](DCCService) for more details.