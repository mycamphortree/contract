这是一个eos的竞猜合约，xiaomiantuan合约是股票合约，最大发行10000。
查看可用命令：
cleos get table xiaomiantuan MT stat
{
  "rows": [{
      "supply": "160.0000 MT",
      "max_supply": "10000.0000 MT",
      "issuer": "xiaomiantuan"
    }
  ],
  "more": false
}

入股：
cleos transfer user xiaomiantuan '10 SYS' 'memo' -p user
查看股权分配:
cleos get table xiaomiantuan MT stat

现行兑换比例：1EOS：1MT

分红：按持股比例分红

注：此合约尚在研发阶段

微信：huangyuancongyu
