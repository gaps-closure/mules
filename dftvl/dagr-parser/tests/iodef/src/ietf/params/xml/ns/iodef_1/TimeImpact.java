//
// This file was generated by the JavaTM Architecture for XML Binding(JAXB) Reference Implementation, v2.2.8-b130911.1802 
// See <a href="http://java.sun.com/xml/jaxb">http://java.sun.com/xml/jaxb</a> 
// Any modifications to this file will be lost upon recompilation of the source schema. 
// Generated on: 2021.11.03 at 01:43:27 PM EDT 
//


package ietf.params.xml.ns.iodef_1;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;
import javax.xml.bind.annotation.XmlValue;
import javax.xml.bind.annotation.adapters.CollapsedStringAdapter;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;


/**
 * <p>Java class for anonymous complex type.
 * 
 * <p>The following schema fragment specifies the expected content contained within this class.
 * 
 * <pre>
 * &lt;complexType>
 *   &lt;simpleContent>
 *     &lt;extension base="&lt;urn:ietf:params:xml:ns:iodef-1.0>PositiveFloatType">
 *       &lt;attribute name="severity" type="{urn:ietf:params:xml:ns:iodef-1.0}severity-type" />
 *       &lt;attribute name="metric" use="required">
 *         &lt;simpleType>
 *           &lt;restriction base="{http://www.w3.org/2001/XMLSchema}NMTOKEN">
 *             &lt;enumeration value="labor"/>
 *             &lt;enumeration value="elapsed"/>
 *             &lt;enumeration value="downtime"/>
 *             &lt;enumeration value="ext-value"/>
 *           &lt;/restriction>
 *         &lt;/simpleType>
 *       &lt;/attribute>
 *       &lt;attribute name="ext-metric" type="{http://www.w3.org/2001/XMLSchema}string" />
 *       &lt;attribute name="duration" type="{urn:ietf:params:xml:ns:iodef-1.0}duration-type" />
 *       &lt;attribute name="ext-duration" type="{http://www.w3.org/2001/XMLSchema}string" />
 *     &lt;/extension>
 *   &lt;/simpleContent>
 * &lt;/complexType>
 * </pre>
 * 
 * 
 */
@XmlAccessorType(XmlAccessType.FIELD)
@XmlType(name = "", propOrder = {
    "value"
})
@XmlRootElement(name = "TimeImpact")
public class TimeImpact {

    @XmlValue
    protected float value;
    @XmlAttribute(name = "severity")
    protected SeverityType severity;
    @XmlAttribute(name = "metric", required = true)
    @XmlJavaTypeAdapter(CollapsedStringAdapter.class)
    protected String metric;
    @XmlAttribute(name = "ext-metric")
    protected String extMetric;
    @XmlAttribute(name = "duration")
    protected DurationType duration;
    @XmlAttribute(name = "ext-duration")
    protected String extDuration;

    /**
     * Gets the value of the value property.
     * 
     */
    public float getValue() {
        return value;
    }

    /**
     * Sets the value of the value property.
     * 
     */
    public void setValue(float value) {
        this.value = value;
    }

    /**
     * Gets the value of the severity property.
     * 
     * @return
     *     possible object is
     *     {@link SeverityType }
     *     
     */
    public SeverityType getSeverity() {
        return severity;
    }

    /**
     * Sets the value of the severity property.
     * 
     * @param value
     *     allowed object is
     *     {@link SeverityType }
     *     
     */
    public void setSeverity(SeverityType value) {
        this.severity = value;
    }

    /**
     * Gets the value of the metric property.
     * 
     * @return
     *     possible object is
     *     {@link String }
     *     
     */
    public String getMetric() {
        return metric;
    }

    /**
     * Sets the value of the metric property.
     * 
     * @param value
     *     allowed object is
     *     {@link String }
     *     
     */
    public void setMetric(String value) {
        this.metric = value;
    }

    /**
     * Gets the value of the extMetric property.
     * 
     * @return
     *     possible object is
     *     {@link String }
     *     
     */
    public String getExtMetric() {
        return extMetric;
    }

    /**
     * Sets the value of the extMetric property.
     * 
     * @param value
     *     allowed object is
     *     {@link String }
     *     
     */
    public void setExtMetric(String value) {
        this.extMetric = value;
    }

    /**
     * Gets the value of the duration property.
     * 
     * @return
     *     possible object is
     *     {@link DurationType }
     *     
     */
    public DurationType getDuration() {
        return duration;
    }

    /**
     * Sets the value of the duration property.
     * 
     * @param value
     *     allowed object is
     *     {@link DurationType }
     *     
     */
    public void setDuration(DurationType value) {
        this.duration = value;
    }

    /**
     * Gets the value of the extDuration property.
     * 
     * @return
     *     possible object is
     *     {@link String }
     *     
     */
    public String getExtDuration() {
        return extDuration;
    }

    /**
     * Sets the value of the extDuration property.
     * 
     * @param value
     *     allowed object is
     *     {@link String }
     *     
     */
    public void setExtDuration(String value) {
        this.extDuration = value;
    }

}
