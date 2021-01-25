FROM gcc:10.2.0

RUN apt-get -y update && apt-get install -y

RUN apt-get -y install cmake libgl1-mesa-dev xorg-dev

# WORKDIR /app

# RUN mkdir build && cd build

# WORKDIR /app/build

# RUN cmake ..

# RUN make

# WORKDIR /app/build/apps

# CMD ["./Modeler"]