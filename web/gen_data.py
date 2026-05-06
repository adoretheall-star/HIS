#!/usr/bin/env python3
"""Generate JavaScript initDemoData() from real data files.
Reads data files using the exact field order and delimiter ('|') defined in data_io.c.
Encoding: UTF-8 (verified via repr() inspection). Line endings: CRLF.
"""
import os

DATA_DIR = os.path.join(os.path.dirname(__file__), '..', 'data')

def read_file(name, min_fields=1):
    """Read a pipe-delimited data file, skip header, return list of field lists.
    Handles CRLF line endings by stripping \\r from each field.
    Returns empty list if file does not exist.
    """
    path = os.path.join(DATA_DIR, name)
    if not os.path.exists(path):
        return []
    with open(path, 'r', encoding='utf-8') as f:
        raw = f.read()
    # Normalize line endings
    raw = raw.replace('\r\n', '\n').replace('\r', '\n')
    lines = raw.strip().split('\n')
    if len(lines) < 2:
        return []
    result = []
    for line in lines[1:]:  # skip header
        line = line.strip()
        if not line:
            continue
        fields = [f.strip() for f in line.split('|')]
        if len(fields) >= min_fields:
            result.append(fields)
    return result

def to_int(s, default=0):
    try: return int(s)
    except: return default

def to_float(s, default=0.0):
    try: return float(s)
    except: return default

def to_bool(s):
    """Normalize boolean fields: some data files have timestamps instead of 0/1."""
    try:
        v = int(s)
        return 1 if v != 0 else 0
    except:
        return 0

def status_text(v):
    """MedStatus enum → UI status text."""
    m = {'1':'待诊','2':'检查中','3':'检查后待复诊','4':'待诊','5':'待诊','6':'住院中','7':'已完成','8':'过号作废'}
    return m.get(str(v), '待诊')

def appt_status_text(v):
    """AppointmentStatus: data uses 1=待确认,2=已确认,3=已完成,4=已取消"""
    m = {'1':'已预约','2':'已到诊','3':'已完成','4':'已取消'}
    return m.get(str(v), '已预约')

def ward_type_name(v):
    m = {'1':'普通病房','2':'ICU','3':'隔离病房','4':'单人病房'}
    return m.get(str(v), '普通病房')

def js_str(s):
    """Escape string for safe embedding in JS single-quoted string."""
    return str(s).replace('\\','\\\\').replace("'","\\'").replace('\n','\\n').replace('\r','')

def main():
    # ==================== PATIENTS ====================
    # data_io.c load_patient_list: 27 fields (indices 0..26), delimiter '|'
    # fprintf order: id|name|gender|age|id_card|m_type|symptom|target_dept|doctor_id|card_id|
    #   balance|status|diagnosis_text|treatment_advice|prescription_str|script_count|
    #   missed_time_1|missed_time_2|missed_time_3|missed_count|blacklist_expire|
    #   is_blacklisted|is_emergency|queue_time|call_count|emergency_debt|unpaid_time
    patients_raw = read_file('patients.txt', min_fields=27)
    patients = []
    for r in patients_raw:
        p = {
            'id': r[0], 'name': r[1], 'gender': r[2],
            'age': to_int(r[3]), 'idCard': r[4], 'm_type': to_int(r[5]),
            'complaint': r[6], 'target_dept': r[7], 'doctor_id': r[8],
            'card_id': r[9], 'balance': to_float(r[10]),
            'status': status_text(r[11]),
            'diagnosis': r[12], 'treatment': r[13],
            'prescription_str': r[14],
            'emergency': to_bool(r[22]),
            'blacklist': to_bool(r[21]),
        }
        patients.append(p)

    # ==================== DOCTORS ====================
    # data_io.c: id|name|gender|department|queue_length|is_on_duty  (6 fields)
    doctors_raw = read_file('doctors.txt', min_fields=6)
    doctors = []
    for r in doctors_raw:
        d = {
            'id': r[0], 'name': r[1], 'gender': r[2], 'dept': r[3],
            'queue_length': to_int(r[4]), 'onDuty': to_int(r[5]),
            'title': '主治医师', 'regFee': 30, 'remainSlots': 20,
        }
        doctors.append(d)

    # ==================== DEPARTMENTS (derived) ====================
    depts_set = set()
    for d in doctors:
        depts_set.add(d['dept'])
    wards_raw = read_file('wards.txt', min_fields=6)
    for w in wards_raw:
        if w[3]:
            depts_set.add(w[3])
    depts = []
    for i, name in enumerate(sorted(depts_set)):
        depts.append({
            'id': 'DEPT{:02d}'.format(i+1), 'name': name,
            'location': '', 'head': '', 'desc': '',
        })

    # ==================== MEDICINES ====================
    # data_io.c: id|name|alias|generic_name|price|stock|m_type|expiry_date (8 fields)
    meds_raw = read_file('medicines.txt', min_fields=8)
    medicines = []
    for r in meds_raw:
        m = {
            'id': r[0], 'name': r[1], 'alias': r[2], 'generic_name': r[3],
            'price': to_float(r[4]), 'stock': to_int(r[5]),
            'm_type': to_int(r[6]), 'expiry': r[7],
            'category': r[3] if r[3] and len(r[3]) < 30 else r[2],
            'manufacturer': '',
        }
        medicines.append(m)

    # ==================== CHECK ITEMS ====================
    # data_io.c: item_id|item_name|dept|price|m_type (5 fields)
    ci_raw = read_file('check_items.txt', min_fields=5)
    check_items = []
    for r in ci_raw:
        check_items.append({
            'item_id': r[0], 'item_name': r[1], 'dept': r[2],
            'price': to_float(r[3]), 'm_type': to_int(r[4]),
        })

    # ==================== CHECK RECORDS ====================
    # data_io.c: record_id|patient_id|item_id|item_name|dept|check_time|result|is_completed|is_paid (9 fields)
    cr_raw = read_file('check_records.txt', min_fields=9)
    check_records = []
    for r in cr_raw:
        check_records.append({
            'record_id': r[0], 'patient_id': r[1], 'item_id': r[2],
            'item_name': r[3], 'dept': r[4], 'check_time': r[5],
            'result': r[6], 'is_completed': to_int(r[7]), 'is_paid': to_int(r[8]),
        })

    # ==================== ACCOUNTS ====================
    # data_io.c: username|password|real_name|gender|role|error_count|lock_time|is_on_duty (8 fields)
    acc_raw = read_file('accounts.txt', min_fields=8)
    accounts = []
    for r in acc_raw:
        accounts.append({
            'username': r[0], 'password': r[1], 'real_name': r[2],
            'gender': r[3], 'role': to_int(r[4]),
            'error_count': to_int(r[5]), 'lock_time': to_int(r[6]),
            'is_on_duty': to_int(r[7]),
        })

    # ==================== WARDS ====================
    # data_io.c: room_id|bed_id|ward_type|dept|is_occupied|patient_id (6 fields)
    wards = []
    for w in wards_raw:
        wt = to_int(w[2], 1)
        wards.append({
            'room_id': w[0], 'bed_id': w[1], 'ward_type': wt,
            'ward_type_name': ward_type_name(wt),
            'dept': w[3] if len(w) > 3 else '',
            'is_occupied': to_int(w[4]) if len(w) > 4 else 0,
            'patient_id': w[5] if len(w) > 5 else '',
        })

    # ==================== APPOINTMENTS ====================
    # data_io.c: appointment_id|patient_id|date|slot|doctor_id|department|status|fee|fee_paid|is_walk_in (10 fields)
    # Deduplicate by appointment_id (file has copies for different date groups)
    appt_raw = read_file('appointments.txt', min_fields=10)
    seen_appt = set()
    appointments = []
    for r in appt_raw:
        aid = r[0]
        if aid in seen_appt:
            continue
        seen_appt.add(aid)
        pname = ''
        for p in patients:
            if p['id'] == r[1]:
                pname = p['name']
                break
        dname = ''
        ddept = r[5]
        for d in doctors:
            if d['id'] == r[4]:
                dname = d['name']
                ddept = d['dept']
                break
        appointments.append({
            'id': aid, 'patientId': r[1], 'patientName': pname,
            'dept': ddept, 'doctorId': r[4], 'doctorName': dname,
            'apptDate': r[2], 'apptSlot': r[3],
            'fee': to_float(r[7]),
            'status': appt_status_text(r[6]),
            'feePaid': to_int(r[8]), 'isWalkIn': to_int(r[9]),
        })

    # ==================== INPATIENTS ====================
    # data_io.c: inpatient_id|patient_id|bed_id|original_bed_id|ward_type|
    #   recommended_ward_type|estimated_days|days_stayed|deposit_balance|is_active|last_settlement_time (11 fields)
    inp_raw = read_file('inpatients.txt', min_fields=11)
    inpatients = []
    for r in inp_raw:
        pname = ''
        target_dept = ''
        doctor_id = ''
        for p in patients:
            if p['id'] == r[1]:
                pname = p['name']
                target_dept = p.get('target_dept', '')
                doctor_id = p.get('doctor_id', '')
                break
        dname = ''
        for d in doctors:
            if d['id'] == doctor_id:
                dname = d['name']
                break
        ip = {
            'id': r[0], 'patientId': r[1], 'patientName': pname,
            'dept': target_dept, 'doctorName': dname,
            'ward': r[2], 'bed': r[2],
            'admitTime': r[10] if len(r) > 10 and r[10] != '未日结' else '',
            'dischargeTime': '',
            'status': '住院中' if to_int(r[9]) else '已出院',
            'fee': 0, 'deposit': to_float(r[8]),
        }
        inpatients.append(ip)

    # ==================== CONSULT RECORDS ====================
    # data_io.c: record_id|patient_id|doctor_id|appointment_id|consult_time|
    #   diagnosis_text|treatment_advice|decision|pre_status|post_status|star_rating|feedback (12 fields)
    con_raw = read_file('consult_records.txt', min_fields=12)
    consult_records = []
    for r in con_raw:
        pname = ''
        for p in patients:
            if p['id'] == r[1]:
                pname = p['name']
                break
        dname = ''
        for d in doctors:
            if d['id'] == r[2]:
                dname = d['name']
                break
        consult_records.append({
            'recordId': r[0], 'patientId': r[1], 'patientName': pname,
            'doctorName': dname, 'diagnosis': r[5], 'advice': r[6],
            'time': r[4], 'doctorId': r[2], 'appointmentId': r[3],
            'decision': to_int(r[7]), 'starRating': to_int(r[10]),
            'feedback': r[11] if len(r) > 11 else '',
        })

    # ==================== PRESCRIPTIONS (from patient data) ====================
    rx_idx = 0
    rx_data = []  # store tuples for later JS generation
    for r in patients_raw:
        pid = r[0]
        pname = r[1]
        rx_str = r[14] if len(r) > 14 else ''
        if not rx_str or rx_str == 'EMPTY' or rx_str == '0':
            continue
        # Parse "M-001:2,M-003:1" format
        parts = rx_str.replace('，', ',').split(',')
        meds = []
        total = 0.0
        for part in parts:
            part = part.strip()
            if ':' not in part:
                continue
            mid, qty_str = part.split(':', 1)
            mid = mid.strip()
            qty = to_int(qty_str.strip(), 1)
            mname = mid
            mprice = 0.0
            for m in medicines:
                if m['id'] == mid:
                    mname = m['name']
                    mprice = m['price']
                    break
            meds.append({'medId': mid, 'medName': mname, 'qty': qty, 'price': mprice})
            total += mprice * qty
        if meds:
            rx_idx += 1
            dname = ''
            for d in doctors:
                if d['id'] == r[8]:
                    dname = d['name']
                    break
            rx_data.append({
                'rxid': 'RX{:03d}'.format(rx_idx),
                'pid': pid, 'pname': pname, 'dname': dname,
                'meds': meds, 'total': total,
            })

    # ==================== LOGS ====================
    # data_io.c: timestamp|operation|target|description (4 fields)
    logs_raw = read_file('logs.txt', min_fields=4)

    # ==================== ALERTS ====================
    # data_io.c: message|time (2 fields, but uses strrchr for parsing)
    alerts_raw = read_file('alerts.txt', min_fields=2)

    # ==================== COMPLAINTS ====================
    # data_io.c: complaint_id|patient_id|target_type|target_id|target_name|content|status|response|submit_time (9 fields)
    complaints_raw = read_file('complaints.txt', min_fields=9)

    # ==================== RECYCLE ====================
    recycle_raw = read_file('recycle.txt', min_fields=0)

    # ==================== GENERATE JAVASCRIPT ====================
    lines = []
    lines.append('function initDemoData(){')

    # Patients
    lines.append('  var ps=[')
    for i, p in enumerate(patients):
        comma = ',' if i < len(patients)-1 else ''
        obj = "{{id:'{}',name:'{}',gender:'{}',age:{},phone:'',idCard:'{}',address:'',complaint:'{}',status:'{}',balance:{},emergency:{},blacklist:{}}}".format(
            js_str(p['id']), js_str(p['name']), p['gender'], p['age'],
            js_str(p['idCard']), js_str(p['complaint']), p['status'],
            p['balance'], p['emergency'], p['blacklist'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SP(ps);')

    # Doctors
    lines.append('  var ds=[')
    for i, d in enumerate(doctors):
        comma = ',' if i < len(doctors)-1 else ''
        obj = "{{id:'{}',name:'{}',gender:'{}',dept:'{}',title:'{}',onDuty:{},regFee:{},remainSlots:{}}}".format(
            js_str(d['id']), js_str(d['name']), d['gender'], js_str(d['dept']),
            d['title'], d['onDuty'], d['regFee'], d['remainSlots'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SD(ds);')

    # Departments
    lines.append('  var deps=[')
    for i, d in enumerate(depts):
        comma = ',' if i < len(depts)-1 else ''
        obj = "{{id:'{}',name:'{}',location:'',head:'',desc:''}}".format(
            js_str(d['id']), js_str(d['name']))
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SE(deps);')

    # Medicines
    lines.append('  var ms=[')
    for i, m in enumerate(medicines):
        comma = ',' if i < len(medicines)-1 else ''
        obj = "{{id:'{}',name:'{}',category:'{}',price:{},stock:{},manufacturer:'',expiry:'{}'}}".format(
            js_str(m['id']), js_str(m['name']), js_str(m['category']),
            m['price'], m['stock'], m['expiry'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SM(ms);')

    # Appointments
    lines.append('  var apts=[')
    for i, a in enumerate(appointments):
        comma = ',' if i < len(appointments)-1 else ''
        obj = "{{id:'{}',patientId:'{}',patientName:'{}',dept:'{}',doctorId:'{}',doctorName:'{}',apptDate:'{}',apptSlot:'{}',fee:{},status:'{}'}}".format(
            js_str(a['id']), js_str(a['patientId']), js_str(a['patientName']),
            js_str(a['dept']), js_str(a['doctorId']), js_str(a['doctorName']),
            a['apptDate'], a['apptSlot'], a['fee'], a['status'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SA(apts);')

    # Registrations + Wait Queue (derived from checked-in appointments)
    lines.append('  var regs=[],wq=[];')
    lines.append('  apts.forEach(function(a,idx){')
    lines.append('    if(a.status==="已到诊"){')
    lines.append('      var rid="REG"+String(regs.length+1).padStart(3,"0");')
    lines.append('      var qno=1;wq.forEach(function(w){if(w.doctorName===a.doctorName)qno++;});')
    lines.append('      regs.push({id:rid,patientId:a.patientId,patientName:a.patientName,dept:a.dept,doctorId:a.doctorId,doctorName:a.doctorName,fee:a.fee,time:"2026-05-05 08:00:00",status:"候诊中",regType:a.isWalkIn?"现场号":"预约转挂号",queueNo:qno});')
    lines.append('      wq.push({queueId:"Q"+String(wq.length+1).padStart(3,"0"),regId:rid,patientId:a.patientId,patientName:a.patientName,dept:a.dept,doctorName:a.doctorName,queueNo:qno,status:"候诊中"});')
    lines.append('    }')
    lines.append('  });')
    lines.append('  SR(regs);SW(wq);')

    # Inpatients
    lines.append('  var ips=[')
    for i, ip in enumerate(inpatients):
        comma = ',' if i < len(inpatients)-1 else ''
        obj = "{{id:'{}',patientId:'{}',patientName:'{}',dept:'{}',doctorName:'{}',ward:'{}',bed:'{}',admitTime:'{}',dischargeTime:'',status:'{}',fee:0,deposit:{}}}".format(
            js_str(ip['id']), js_str(ip['patientId']), js_str(ip['patientName']),
            js_str(ip['dept']), js_str(ip['doctorName']), js_str(ip['ward']),
            js_str(ip['bed']), js_str(ip['admitTime']), ip['status'], ip['deposit'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SI(ips);')

    # Prescriptions
    lines.append('  var rxs=[];')
    for rx in rx_data:
        med_objs = ','.join([
            "{{medId:'{}',medName:'{}',qty:{},price:{}}}".format(
                m['medId'], js_str(m['medName']), m['qty'], m['price'])
            for m in rx['meds']
        ])
        lines.append("  rxs.push({{id:'{}',regId:'',patientId:'{}',patientName:'{}',doctorName:'{}',medicines:[{}],total:{:.2f},time:'2026-04-15 09:00:00',status:'已发药'}});".format(
            rx['rxid'], rx['pid'], js_str(rx['pname']), js_str(rx['dname']),
            med_objs, rx['total']))
    lines.append('  SX(rxs);')

    # Billings
    lines.append('  var bills=[];')
    lines.append('  rxs.forEach(function(rx){bills.push({id:"BILL"+String(bills.length+1).padStart(3,"0"),type:"药品费",patientId:rx.patientId,patientName:rx.patientName,amount:rx.total,time:rx.time,status:"已收费",refId:rx.id});});')
    lines.append('  ips.forEach(function(ip){if(ip.deposit>0){bills.push({id:"BILL"+String(bills.length+1).padStart(3,"0"),type:"住院押金",patientId:ip.patientId,patientName:ip.patientName,amount:ip.deposit,time:ip.admitTime,status:"已收费",refId:ip.id});}});')
    lines.append('  SB(bills);')

    # Consult Records
    lines.append('  var crs=[')
    for i, cr in enumerate(consult_records):
        comma = ',' if i < len(consult_records)-1 else ''
        obj = "{{recordId:'{}',patientId:'{}',patientName:'{}',doctorName:'{}',diagnosis:'{}',advice:'{}',time:'{}'}}".format(
            js_str(cr['recordId']), js_str(cr['patientId']), js_str(cr['patientName']),
            js_str(cr['doctorName']), js_str(cr['diagnosis']), js_str(cr['advice']),
            cr['time'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  SC(crs);')

    # Check Items → localStorage
    lines.append('  var cis=[')
    for i, ci in enumerate(check_items):
        comma = ',' if i < len(check_items)-1 else ''
        obj = "{{id:'{}',name:'{}',dept:'{}',price:{},mType:{}}}".format(
            js_str(ci['item_id']), js_str(ci['item_name']), js_str(ci['dept']),
            ci['price'], ci['m_type'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_checkitems",JSON.stringify(cis));')

    # Check Records → localStorage
    lines.append('  var chkrecs=[')
    for i, cr in enumerate(check_records):
        comma = ',' if i < len(check_records)-1 else ''
        obj = "{{recordId:'{}',patientId:'{}',itemId:'{}',itemName:'{}',dept:'{}',checkTime:'{}',result:'{}',isCompleted:{},isPaid:{}}}".format(
            js_str(cr['record_id']), js_str(cr['patient_id']), js_str(cr['item_id']),
            js_str(cr['item_name']), js_str(cr['dept']), cr['check_time'],
            js_str(cr['result']), cr['is_completed'], cr['is_paid'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_checkrecords",JSON.stringify(chkrecs));')

    # Accounts → localStorage
    lines.append('  var accs=[')
    for i, a in enumerate(accounts):
        comma = ',' if i < len(accounts)-1 else ''
        obj = "{{username:'{}',password:'{}',realName:'{}',gender:'{}',role:{},errorCount:{},lockTime:{},isOnDuty:{}}}".format(
            js_str(a['username']), js_str(a['password']), js_str(a['real_name']),
            a['gender'], a['role'], a['error_count'], a['lock_time'], a['is_on_duty'])
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_accounts",JSON.stringify(accs));')

    # Wards → localStorage
    lines.append('  var wardData=[')
    for i, w in enumerate(wards):
        comma = ',' if i < len(wards)-1 else ''
        obj = "{{roomId:'{}',bedId:'{}',wardType:{},wardTypeName:'{}',dept:'{}',isOccupied:{},patientId:'{}'}}".format(
            js_str(w['room_id']), js_str(w['bed_id']), w['ward_type'],
            w['ward_type_name'], js_str(w['dept']), w['is_occupied'],
            js_str(w['patient_id']))
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_wards",JSON.stringify(wardData));')

    # Alerts → localStorage
    lines.append('  var alertData=[')
    for i, a in enumerate(alerts_raw):
        comma = ',' if i < len(alerts_raw)-1 else ''
        obj = "{{message:'{}',time:'{}'}}".format(js_str(a[0]), a[1] if len(a) > 1 else '')
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_alerts",JSON.stringify(alertData));')

    # Complaints → localStorage
    lines.append('  var complaintData=[')
    for i, c in enumerate(complaints_raw):
        comma = ',' if i < len(complaints_raw)-1 else ''
        obj = "{{complaintId:'{}',patientId:'{}',targetType:{},targetId:'{}',targetName:'{}',content:'{}',status:{},response:'{}',submitTime:'{}'}}".format(
            js_str(c[0]), js_str(c[1]), to_int(c[2]), js_str(c[3]),
            js_str(c[4]), js_str(c[5]), to_int(c[6]),
            js_str(c[7]) if len(c) > 7 else '',
            c[8] if len(c) > 8 else '')
        lines.append('    '+obj+comma)
    lines.append('  ];')
    lines.append('  localStorage.setItem("his_complaints",JSON.stringify(complaintData));')

    # Logs
    lines.append('  SL([]);')
    lines.append('  addLog("系统初始化","全部模块","从data/目录加载真实数据初始化完成");')

    # Summary toast
    total_inpatient_records = len(inpatients)
    summary = '真实数据初始化成功！患者{} 医生{} 科室{} 药品{} 预约{} 住院{} 接诊记录{} 检查项目{} 检查记录{} 账号{} 床位{}'.format(
        len(patients), len(doctors), len(depts), len(medicines),
        len(appointments), total_inpatient_records, len(consult_records),
        len(check_items), len(check_records), len(accounts), len(wards))
    lines.append("  toast('"+summary+"');")
    lines.append('}')

    output = '\n'.join(lines)

    outpath = os.path.join(os.path.dirname(__file__), 'init_demo_data.js')
    with open(outpath, 'w', encoding='utf-8') as f:
        f.write(output)

    # Print stats
    print("Generated initDemoData() function")
    print("  Patients:         {}".format(len(patients)))
    print("  Doctors:          {}".format(len(doctors)))
    print("  Departments:      {}".format(len(depts)))
    print("  Medicines:        {}".format(len(medicines)))
    print("  Appointments:     {} (deduped)".format(len(appointments)))
    print("  Inpatients:       {}".format(len(inpatients)))
    print("  Consult Records:  {}".format(len(consult_records)))
    print("  Check Items:      {}".format(len(check_items)))
    print("  Check Records:    {}".format(len(check_records)))
    print("  Accounts:         {}".format(len(accounts)))
    print("  Wards:            {}".format(len(wards)))
    print("  Alerts:           {}".format(len(alerts_raw)))
    print("  Complaints:       {}".format(len(complaints_raw)))
    print("  Prescriptions:    {}".format(rx_idx))
    print("  Logs (file):      {}".format(len(logs_raw)))
    print("\nOutput: {}".format(outpath))

if __name__ == '__main__':
    main()
