# Test case for DFDL autogeneration and DAGR/DFTVL development

We are developing a testcase based on RFC5070 IODEF. We start with the XSD schema and four XML examples 
for that schema that are provided in this RFC.

```
wget https://www.iana.org/assignments/xml-registry/schema/iodef-1.0.xsd
wget https://www.rfc-editor.org/rfc/rfc5070.txt

``

We have set up a basic ant build file. We have also set up a custom bindings file `addser.xjb` to specify xjc to 
generate the Serializable interface, using the approach from this [link](https://stackoverflow.com/questions/1513972/how-to-generate-a-java-class-which-implements-serializable-interface-from-xsd-us).

Then generate the Java classes from this schema using JAXB.

```
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH

mkdir src lib
xjc iodef-1.0.xsd -b addser.xjb
mv ietf src
```

```
ant 
java -classpath ./dist/IODEF_DAGRDFDLTEST.jar TestDFDLAutogen worm.xml 
```
 

