#include "office.h"

#include <string.h>

#define SIZE sizeof(struct employee)

void addSubordinate(struct employee* pSupervisor, struct employee* pEmployee);
void removeEmployee(struct employee* pEmployee);
void replaceSubordinate(struct employee* pSupervisor, struct employee* pPredecessor, struct employee* pSuccessor);
void freeSingleMem(struct employee* pEmployee);
void freeListMem(struct employee* pList, size_t iLen);
struct employee* push_back(struct employee* pOriginal, size_t nOriginal, struct employee* pEmployee, size_t nEmployee);
size_t determineDepth(struct office* pOffice);
size_t minimum(size_t a, size_t b);

/**
 * If the supervisor field is NULL it is assumed the employee will be placed under the next employee that is
 * not superivising any other employee (top-down, left-to-right).
 *
 * If the supervisor is not NULL, it will be added to the supervisor's subordinates list of employees (make sure the supervisor exists in hierarchy).
 * if the office or employee are null, the function not do anything.
 */
void office_employee_place(struct office* off, struct employee* supervisor, struct employee* emp)
{
	//  If the office or employee is null, the function does nothing
	if (off == NULL || emp == NULL)
		return;

	//  If the supervisor is not null, the employee will be added to the supervisor's suborinates list (reallocate enough memory, copy the employee to the end, and set the supervisor pointer of the employee
	if (supervisor != NULL)
	{
		addSubordinate(supervisor, emp);
		return;
	}

	//  If the office has no head, set the employee as the head
	if (off->department_head == NULL)
	{
		off->department_head = emp;
		return;
	}

	size_t iDepth = determineDepth(off);

	for (size_t i = 1; i < iDepth; i++)
	{
		struct employee* pList = NULL;
		size_t iList = 0;
		office_get_employees_at_level(off, i, &pList, &iList);

		for (size_t j = 0; j < iList; j++)
		{
			if (pList[j].subordinates == NULL)
			{
				addSubordinate(pList + j, emp);
				free(pList);
				return;
			}
		}

		if (pList != NULL)
			free(pList); 
	}
}

/**
 * Fires an employee, removing from the office
 * If employee is null, nothing should occur
 * If the employee does not supervise anyone, they will just be removed
 * If the employee is supervising other employees, the first member of that
 *  team will replace him.
 */
void office_fire_employee(struct employee* employee)
{
	//  If employee is null, nothing should occur
	if (employee == NULL)
		return;

	if (employee->n_subordinates != 0)
	{
		replaceSubordinate(employee->supervisor, employee, employee->subordinates);
	}

	removeEmployee(employee);
}

/**
 * Retrieves the first encounter where the employee's name matches name
 * If the employee does not exist, it must return NULL
 * if office or name are NULL, your function must do nothing
 */
struct employee* office_get_first_employee_with_name(struct office* office, const char* name)
{
	//  If office or name is null, the function does nothing
	if (office != NULL && name != NULL && office->department_head != NULL)
	{
		struct employee* pHead = office->department_head;

		//	If the office head is the one (who with a specific name) you are looking for
		if (strcmp(pHead->name, name) == 0)
			return pHead;

		//	Traverse all subordinates of the office head
		struct employee* pList = office->department_head->subordinates;
		size_t iList = office->department_head->n_subordinates;

		for (size_t i = 0; i < iList; i++)
		{
			if (strcmp(pList[i].name, name) == 0)
				return pList + i;
		}
	}
	return NULL;
}

/**
 * Retrieves the last encounter where the employee's name matches name
 * If the employee does not exist, it must return NULL
 * if office or name are NULL, your function must do nothing
 */
struct employee* office_get_last_employee_with_name(struct office* office, const char* name)
{
	struct employee* pFound = NULL;

	//  If office or name is null, the function does nothing
	if (office != NULL && name != NULL && office->department_head != NULL)
	{
		struct employee* pHead = office->department_head;

		//	If the office head is the one (who with a specific name) you are looking for
		if (strcmp(pHead->name, name) == 0)
			pFound = pHead;

		//	Traverse all subordinates of the office head
		struct employee* pList = office->department_head->subordinates;
		size_t iList = office->department_head->n_subordinates;

		for (size_t i = 0; i < iList; i++)
		{
			if (strcmp(pList[i].name, name) == 0)
				pFound = pList + i;
		}
	}
	return pFound;
}


/**
 * This function will need to retrieve all employees at a level.
 * A level is defined as distance away from the boss. For example, all
 * subordinates of the boss are 1 level away, all subordinates of those
 * subordinates are 2 levels away.
 *
 * if office, n_employees or emplys are NULL, your function must do nothing
 * You will need to provide an allocation to employs and specify the
 * correct number of employees found in your query.
 */
void office_get_employees_at_level(struct office* office, size_t level, struct employee** emplys, size_t* n_employees)
{
	if (office == NULL || emplys == NULL || n_employees == NULL)
		return;

	struct employee* pList = push_back(NULL, 0, office->department_head, 1);
	size_t iList = 1;

	for (size_t i = 0; i < level; i++)
	{
		struct employee* pTemp = NULL;
		size_t iTemp = 0;

		for (size_t j = 0; j < iList; j++)
		{
			if (pList[j].subordinates != NULL)
			{
				pTemp = push_back(pTemp, iTemp, pList[j].subordinates, pList[j].n_subordinates);
				iTemp += pList[i].n_subordinates;
			}
		}

		free(pList);
		pList = pTemp;
		iList = iTemp;
	}

	*emplys = pList;
	*n_employees = iList;
}

/**
 * Will retrieve a list of employees that match the name given
 * If office, name, emplys or n_employees is NULL, this function should do
 * nothing
 * You will need to provide an allocation to employs and specify the
 * correct number of employees found in your query.
 */
void office_get_employees_by_name(struct office* office, const char* name, struct employee** emplys, size_t* n_employees)
{
	if (office == NULL || name == NULL || emplys == NULL || n_employees == NULL)
		return;

	struct employee* pFound = NULL;
	size_t iFound = 0;

	struct employee* pList = office->department_head->subordinates;
	size_t iList = office->department_head->n_subordinates;

	for (size_t i = 0; i < iList; i++)
	{
		if (strcmp(pList[i].name, name) == 0)
		{
			pFound = push_back(pFound, iFound, pList + i, 1);
			iFound++;
		}
	}

	*emplys = pFound;
	*n_employees = iFound;
}

/**
 * You will traverse the office and retrieve employees using a postorder traversal
 * If off, emplys or n_employees is NULL, this function should do nothing
 *
 * You will need to provide an allocation to emplys and specify the
 * correct number of employees found in your query.
 */
void office_get_employees_postorder(struct office* off, struct employee** emplys, size_t* n_employees)
{
	if (off == NULL || emplys == NULL || n_employees == NULL)
		return;

	struct employee* pPostorder = NULL;
	size_t iPostorder = 0;

	size_t iDepth = determineDepth(off);

	for (size_t i = iDepth; i > 0; i--)
	{
		struct employee* pList = NULL;
		size_t iList = 0;

		office_get_employees_at_level(off, i, &pList, &iList);

		push_back(pPostorder, iPostorder, pList, iList);
		iPostorder += iList;
	}

	*emplys = pPostorder;
	*n_employees = iPostorder;
}

/**
 * The office disbands
 * (You will need to free all memory associated with employees attached to
 *   the office and the office itself)
 */
void office_disband(struct office* office)
{
	int iDepth = determineDepth(office);

	for (size_t i = iDepth; i > 0; i--)
	{
		struct employee* pList = NULL;
		size_t iList = 0;

		office_get_employees_at_level(office, i, &pList, &iList);

		for (size_t j = 0; j < iList; j++)
		{
			removeEmployee(pList + j);
		}
	}

	free(office->department_head);
	free(office);
}

////////////////////////////////////////////////////////////////////////////

size_t determineDepth(struct office* pOffice)
{
	if (pOffice == NULL || pOffice->department_head == NULL)
		return 0;

	size_t iDepth = 1;

	struct employee* pList = push_back(NULL, 0, pOffice->department_head, 1);
	size_t iList = 1;

	while (1)
	{
		struct employee* pTemp = NULL;
		size_t iTemp = 0;

		for (size_t i = 0; i < iList; i++)
		{
			if (pList[i].subordinates != NULL)
			{
				pTemp = push_back(pTemp, iTemp, pList[i].subordinates, pList[i].n_subordinates);
				iTemp += pList[i].n_subordinates;
			}
		}

		if (iTemp != 0)
		{
			if (pTemp != NULL)
				free(pTemp);

			iDepth++;
		}
		else
		{
			return iDepth;
		}
	}

	return iDepth;
}

void addSubordinate(struct employee* pSupervisor, struct employee* pEmployee)
{
	if (pSupervisor != NULL && pEmployee != NULL)
	{
		pEmployee->supervisor = pSupervisor;

		struct employee* pList = (struct employee*)malloc(SIZE * (pSupervisor->n_subordinates + 1));

		if (pSupervisor->subordinates != NULL)
		{
			memcpy(pList, pSupervisor->subordinates, SIZE * pSupervisor->n_subordinates);
			memcpy(pList + pSupervisor->n_subordinates, pEmployee, SIZE);
			free(pSupervisor->subordinates);
		}

		pSupervisor->subordinates = pList;
		pSupervisor->n_subordinates++;
	}
}

void removeEmployee(struct employee* pEmployee)
{
	if (pEmployee != NULL)
	{
		if (pEmployee->supervisor != NULL)
		{
			struct employee* pSupervisor = pEmployee->supervisor;
			struct employee* pList = pSupervisor->subordinates;
			size_t iList = pSupervisor->n_subordinates;

			for (size_t i = 0; i < iList; i++)
			{
				if (pList + i == pEmployee)
				{
					struct employee* pTemp = (struct employee*)malloc(SIZE * (iList - 1));
					memcpy(pTemp, pList, SIZE * i);
					memcpy(pTemp + minimum(i, iList - 2), pList + minimum(i + 1, iList - 1), SIZE * (iList - i - 1));
					free(pSupervisor->subordinates);
					pSupervisor->subordinates = pTemp;
					pSupervisor->n_subordinates--;
				}
			}
		}

		freeSingleMem(pEmployee);
	}
}

void replaceSubordinate(struct employee* pSupervisor, struct employee* pPredecessor, struct employee* pSuccessor)
{
	if (pSupervisor != NULL)
	{
		for (size_t i = 0; i < pSupervisor->n_subordinates; i++)
		{
			if (pSupervisor->subordinates + i == pPredecessor)
			{
				memcpy(pSupervisor->subordinates + i, pSuccessor, SIZE);
				break;
			}
		}
	}
}

void freeSingleMem(struct employee* pEmployee)
{
	free(pEmployee);
}

void freeListMem(struct employee* pList, size_t iLen)
{
	for (size_t i = 0; i < iLen; i++)
	{
		freeSingleMem(pList + i);
	}
}

struct employee* push_back(struct employee* pOriginal, size_t nOriginal, struct employee* pEmployee, size_t nEmployee)
{
	struct employee* pNew = (struct employee*)malloc(SIZE * nOriginal + nEmployee);

	if (pOriginal != NULL)
	{
		memcpy(pNew, pOriginal, SIZE * nOriginal);
		free(pOriginal);
	}

	memcpy(pNew + nOriginal, pEmployee, SIZE * nEmployee);
	return pNew;
}

size_t minimum(size_t a, size_t b)
{
	return a > b ? b : a;
}

void office_promote_employee(struct employee* emp) {}
void office_demote_employee(struct employee* supervisor, struct employee* emp) {}