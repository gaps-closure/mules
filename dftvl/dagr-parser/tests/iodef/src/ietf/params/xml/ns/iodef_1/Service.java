//
// This file was generated by the JavaTM Architecture for XML Binding(JAXB) Reference Implementation, v2.2.8-b130911.1802 
// See <a href="http://java.sun.com/xml/jaxb">http://java.sun.com/xml/jaxb</a> 
// Any modifications to this file will be lost upon recompilation of the source schema. 
// Generated on: 2021.11.16 at 04:48:23 PM EST 
//


package ietf.params.xml.ns.iodef_1;

import java.io.Serializable;
import java.math.BigInteger;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;


/**
 * <p>Java class for anonymous complex type.
 * 
 * <p>The following schema fragment specifies the expected content contained within this class.
 * 
 * <pre>
 * &lt;complexType>
 *   &lt;complexContent>
 *     &lt;restriction base="{http://www.w3.org/2001/XMLSchema}anyType">
 *       &lt;sequence>
 *         &lt;choice minOccurs="0">
 *           &lt;element name="Port" type="{http://www.w3.org/2001/XMLSchema}integer"/>
 *           &lt;element name="Portlist" type="{urn:ietf:params:xml:ns:iodef-1.0}PortlistType"/>
 *         &lt;/choice>
 *         &lt;element name="ProtoType" type="{http://www.w3.org/2001/XMLSchema}integer" minOccurs="0"/>
 *         &lt;element name="ProtoCode" type="{http://www.w3.org/2001/XMLSchema}integer" minOccurs="0"/>
 *         &lt;element name="ProtoField" type="{http://www.w3.org/2001/XMLSchema}integer" minOccurs="0"/>
 *         &lt;element ref="{urn:ietf:params:xml:ns:iodef-1.0}Application" minOccurs="0"/>
 *       &lt;/sequence>
 *       &lt;attribute name="ip_protocol" use="required" type="{http://www.w3.org/2001/XMLSchema}integer" />
 *     &lt;/restriction>
 *   &lt;/complexContent>
 * &lt;/complexType>
 * </pre>
 * 
 * 
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "", propOrder = {
    "port",
    "portlist",
    "protoType",
    "protoCode",
    "protoField",
    "application"
})
@XmlRootElement(name = "Service")
public class Service
    implements Serializable
{

    private final static long serialVersionUID = 1L;
    @XmlElement(name = "Port")
    protected BigInteger port;
    @XmlElement(name = "Portlist")
    protected String portlist;
    @XmlElement(name = "ProtoType")
    protected BigInteger protoType;
    @XmlElement(name = "ProtoCode")
    protected BigInteger protoCode;
    @XmlElement(name = "ProtoField")
    protected BigInteger protoField;
    @XmlElement(name = "Application")
    protected SoftwareType application;
    @XmlAttribute(name = "ip_protocol", required = true)
    protected BigInteger ipProtocol;

    /**
     * Gets the value of the port property.
     * 
     * @return
     *     possible object is
     *     {@link BigInteger }
     *     
     */
    public BigInteger getPort() {
        return port;
    }

    /**
     * Sets the value of the port property.
     * 
     * @param value
     *     allowed object is
     *     {@link BigInteger }
     *     
     */
    public void setPort(BigInteger value) {
        this.port = value;
    }

    /**
     * Gets the value of the portlist property.
     * 
     * @return
     *     possible object is
     *     {@link String }
     *     
     */
    public String getPortlist() {
        return portlist;
    }

    /**
     * Sets the value of the portlist property.
     * 
     * @param value
     *     allowed object is
     *     {@link String }
     *     
     */
    public void setPortlist(String value) {
        this.portlist = value;
    }

    /**
     * Gets the value of the protoType property.
     * 
     * @return
     *     possible object is
     *     {@link BigInteger }
     *     
     */
    public BigInteger getProtoType() {
        return protoType;
    }

    /**
     * Sets the value of the protoType property.
     * 
     * @param value
     *     allowed object is
     *     {@link BigInteger }
     *     
     */
    public void setProtoType(BigInteger value) {
        this.protoType = value;
    }

    /**
     * Gets the value of the protoCode property.
     * 
     * @return
     *     possible object is
     *     {@link BigInteger }
     *     
     */
    public BigInteger getProtoCode() {
        return protoCode;
    }

    /**
     * Sets the value of the protoCode property.
     * 
     * @param value
     *     allowed object is
     *     {@link BigInteger }
     *     
     */
    public void setProtoCode(BigInteger value) {
        this.protoCode = value;
    }

    /**
     * Gets the value of the protoField property.
     * 
     * @return
     *     possible object is
     *     {@link BigInteger }
     *     
     */
    public BigInteger getProtoField() {
        return protoField;
    }

    /**
     * Sets the value of the protoField property.
     * 
     * @param value
     *     allowed object is
     *     {@link BigInteger }
     *     
     */
    public void setProtoField(BigInteger value) {
        this.protoField = value;
    }

    /**
     * Gets the value of the application property.
     * 
     * @return
     *     possible object is
     *     {@link SoftwareType }
     *     
     */
    public SoftwareType getApplication() {
        return application;
    }

    /**
     * Sets the value of the application property.
     * 
     * @param value
     *     allowed object is
     *     {@link SoftwareType }
     *     
     */
    public void setApplication(SoftwareType value) {
        this.application = value;
    }

    /**
     * Gets the value of the ipProtocol property.
     * 
     * @return
     *     possible object is
     *     {@link BigInteger }
     *     
     */
    public BigInteger getIpProtocol() {
        return ipProtocol;
    }

    /**
     * Sets the value of the ipProtocol property.
     * 
     * @param value
     *     allowed object is
     *     {@link BigInteger }
     *     
     */
    public void setIpProtocol(BigInteger value) {
        this.ipProtocol = value;
    }

}