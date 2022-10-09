Simple Cloud Storage. \
Request: \
- /imports: send array of jsons with file/folder info, POST \
- /nodes{id}: return json of file/folder, GET \
- /delete{id}: delete file/folder, DELETE \

Deploy with installed docker: \
  git clone https://github.com/ekarpukhin/my_cloud \
  cd my_cloud \
  sudo docker build -t my_cloud . \
  sudo docker run -p 80:80 --restart=always my_cloud \
Enjoy! \
\
If you need to clear database, send /clear. \
If you need to stop server, send /stop (it will reload automatically). \
\
P.S. Colab with https://github.com/Kostyak7 
