# Test case for DFDL autogeneration and DAGR/DFTVL development

We are developing a testcase based on RFC5070 IODEF. We start with the XSD schema and four XML examples 
for that schema that are provided in this RFC.

```
wget https://www.iana.org/assignments/xml-registry/schema/iodef-1.0.xsd
wget https://www.rfc-editor.org/rfc/rfc5070.txt

``

Then generate the Java classes from this schema using JAXB

```
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH

mkdir src lib
xjc iodef.xsd
mv ietf src
```

```
ant 
java -classpath ./dist/IODEF_DAGRDFDLTEST.jar TestDFDLAutogen worm.xml 
```
 

