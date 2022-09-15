FROM drogonframework/drogon
ADD src .
RUN  &&\
    cd src/build &&\
    cmake .. &&\
    make \
WORKDIR src/build
ENTRYPOINT ["./yashka"]
