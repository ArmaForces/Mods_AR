class AFM_PlainTextContainerSerializer
{
	string SerializeContainer(BaseContainer container, int level = 0)
	{
		string output = SCR_StringHelper.PadLeft("", level*2, SCR_StringHelper.SPACE);
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
	
	string SerializeProp(BaseContainer container, string var, DataVarType type, int level = 0)
	{
		string output = SCR_StringHelper.PadLeft("", level*2, SCR_StringHelper.SPACE);
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
			case DataVarType.INTEGER:
			{
				float val;
				container.Get(var, val);
				output += string.Format("%1: %2", var, val);
				
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
}