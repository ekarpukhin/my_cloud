FROM drogonframework/drogon
COPY src .
RUN ls
RUN cd build &&\
    cmake .. &&\
    make
WORKDIR build
ENTRYPOINT ["./yashka"]
