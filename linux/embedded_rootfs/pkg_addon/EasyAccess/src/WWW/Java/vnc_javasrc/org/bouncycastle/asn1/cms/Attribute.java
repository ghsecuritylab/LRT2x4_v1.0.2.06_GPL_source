package org.bouncycastle.asn1.cms;

import org.bouncycastle.asn1.*;

public class Attribute
    implements DEREncodable
{
	private DERObjectIdentifier attrType;
	private ASN1Set             attrValues;

    /**
     * return an Attribute object from the given object.
     *
     * @param o the object we want converted.
     * @exception IllegalArgumentException if the object cannot be converted.
     */
	public static Attribute getInstance(
        Object o)
    {
		if (o == null || o instanceof Attribute)
        {
			return (Attribute)o;
		}
		
		if (o instanceof ASN1Sequence)
        {
			return new Attribute((ASN1Sequence)o);
		}

        throw new IllegalArgumentException("unknown object in factory");
	}
	
	public Attribute(
        ASN1Sequence seq)
    {
		attrType = (DERObjectIdentifier)seq.getObjectAt(0);
		attrValues = (ASN1Set)seq.getObjectAt(1);
	}

	public Attribute(
	    DERObjectIdentifier attrType,
	    ASN1Set             attrValues)
    {
		this.attrType = attrType;
		this.attrValues = attrValues;
	}

	public DERObjectIdentifier getAttrType()
    {
		return attrType;
	}
	
	public ASN1Set getAttrValues()
    {
		return attrValues;
	}

    /** 
     * <pre>
     * Attribute ::= SEQUENCE {
     * 	attrType OBJECT IDENTIFIER,
     * 	attrValues SET OF AttributeValue
     * }
     * </pre>
     */
	public DERObject getDERObject()
    {
		DEREncodableVector v = new DEREncodableVector();

		v.add(attrType);
		v.add(attrValues);

		return new DERSequence(v);
	}
}
