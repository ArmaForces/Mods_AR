class AFM_PlainTextContainerSerializer
{
	//------------------------------------------------------------------------------------------------
	//! Serializes container into a human readable string
	string SerializeContainer(BaseContainer container, int level = 0)
	{
		string output = MakeIndent(level);
		if (!container)
		{
			output += "null";
			return output;
		}
		
		output += string.Format("%1:", container.GetClassName());
		for (int i, count = container.GetNumVars(); i < count; i++)
		{
			string var = container.GetVarName(i);
			DataVarType type = container.GetDataVarType(i);
			output += "\n";
			output += SerializeProp(container, var, type, level + 1);
		}
		
		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Serializes container property/entry into a human readable string
	string SerializeProp(BaseContainer container, string var, DataVarType type, int level = 0)
	{
		string output = MakeIndent(level);
		switch (type)
		{
			case DataVarType.BOOLEAN:
			{
				bool val;
				container.Get(var, val);
				if (val)
					output += string.Format("%1: true", var);
				else
					output += string.Format("%1: false", var);
				
				break;
			}
			
			case DataVarType.SCALAR:
			{
				float val;
				container.Get(var, val);
				output += string.Format("%1: %2", var, val.ToString(-1, 2));
				
				break;
			}
			
			case DataVarType.INTEGER:
			{
				int val;
				container.Get(var, val);
				output += string.Format("%1: %2", var, val);
				
				break;
			}
			
			case DataVarType.STRING:
			{
				string val;
				container.Get(var, val);
				output += string.Format("%1: %2", var, string.ToString(val));
				
				break;
			}
			
			case DataVarType.RESOURCE_NAME:
			{
				ResourceName val;
				container.Get(var, val);
				output += string.Format("%1: %2", var, val);
				
				break;
			}
			
			case DataVarType.OBJECT:
			{
				BaseContainer containerObject = container.GetObject(var);
				string val = SerializeContainer(containerObject, level + 1);
				output += string.Format("%1:\n%2", var, val);
				
				break;
			}
			
			case DataVarType.OBJECT_ARRAY:
			{
				BaseContainerList list = container.GetObjectArray(var);
				
				for (int i, count = list.Count(); i < count; i++)
				{
					BaseContainer containerEntry = list.Get(i);
					string val = SerializeContainer(containerEntry, level + 1);
					output += string.Format("%1:\n%2", var, val);
				}
				
				break;
			}
			
			default:
			{
				string eType = SCR_Enum.GetEnumName(DataVarType, type);
				output += string.Format("%1: <unhandled type, %2>", var, eType);
			}
		}
		
		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string MakeIndent(int level)
	{
		return SCR_StringHelper.PadLeft("", level*4, SCR_StringHelper.SPACE);
	}
}
