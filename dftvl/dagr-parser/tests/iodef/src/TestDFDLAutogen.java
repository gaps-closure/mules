import javax.xml.bind.*;
import javax.xml.bind.annotation.*;
import javax.xml.bind.annotation.adapters.*;
import ietf.params.xml.ns.iodef_1.*;
import java.io.File;
import java.lang.System;

class TestDFDLAutogen {
  public static void main(String args[]) {
    JAXBContext jaxbContext;
    for (String elem: args) {
    try
      {
        File xmlFile = new File(elem);
        jaxbContext = JAXBContext.newInstance(IODEFDocument.class);              
        Unmarshaller jaxbUnmarshaller = jaxbContext.createUnmarshaller();
        IODEFDocument idoc = (IODEFDocument) jaxbUnmarshaller.unmarshal(xmlFile);
        // XXX: Generate DFDL for IODEFDocument
        // XXX: Serialize object and dump binary
        System.out.println(idoc);
      }
      catch (JAXBException e) 
      {
        e.printStackTrace();
      }
    }
  }
}

