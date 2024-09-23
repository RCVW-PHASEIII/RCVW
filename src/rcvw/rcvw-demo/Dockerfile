
FROM php:8.3-apache
WORKDIR /var/www/html

RUN apt-get update && apt-get install -y gnupg2
RUN curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
RUN curl https://packages.microsoft.com/config/debian/11/prod.list > /etc/apt/sources.list.d/mssql-release.list

RUN apt-get update 

RUN ACCEPT_EULA=Y apt-get -y --no-install-recommends install msodbcsql18

RUN apt-get install -y gpg unixodbc unixodbc-dev 
RUN docker-php-ext-install pdo
RUN pecl install sqlsrv pdo_sqlsrv json

RUN docker-php-ext-enable sqlsrv pdo_sqlsrv 

COPY ./ .

EXPOSE 80